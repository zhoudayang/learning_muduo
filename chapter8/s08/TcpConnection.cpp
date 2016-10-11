//
// Created by fit on 16-10-9.
//

#include "TcpConnection.h"

#include "base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Sockets.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr,
        const InetAddress& peerAddr)
        :loop_(CHECK_NOTNULL(loop)),
         name_(name),
         state_(kConnecting),
         socket_(new Socket(sockfd)),
         channel_(new Channel(loop, sockfd)),
         localAddr_(localAddr),
         peerAddr_(peerAddr)
{
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at"
              << this << " fd=" << sockfd;
    //set read callback function for channel of this TcpConnection
    channel_->setReadCallback(boost::bind(&TcpConnection::handleRead, this, _1));
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at" << this << " fd=" << channel_->fd();
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_==kConnecting);
    setState(kConnected);
    channel_->enableReading();
    //call connection callback function
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    // 使用inputBuffer_来读取数据
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n>0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n==0)
    {
        handleClose();
    }
    else
    {
        //set errno
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }

}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError[" << name_ << "] -SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    //当前连接已经建立，但是对方想要关闭连接或者本方想要关闭连接
    assert(state_==kConnected || state_==kDisconnecting);
    //we don't close fd, leave it to dtor, so we can find leaks easily
    channel_->disableAll();
    //call close callback function in TcpServer
    closeCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    assert(state_==kConnected || state_==kDisconnecting);
    setState(kDisconnected);
    //这里与handleClose重复，这是因为某些情况下可以不经过handleClose 直接调用connecteDestroyed 函数
    channel_->disableAll();
    //call connection callback function, now state is Disconnected
    connectionCallback_(shared_from_this());
    //remove channel from poller
    loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::send(const muduo::string& message)
{
    //如果当前状态是已经连接
    if (state_==kConnected)
    {
        //在IO线程中直接调用sendInLoop函数
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
            //否则延后到IO线程调用
        else
        {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}
/*
 sendInLoop()会先尝试直接发送数据，如果一次发送完毕就不会启用WriteCallback,如果只是发送了
 部分数据，就把剩余数据放入outputBuffer_,并且开始关注writable事件，以后在handleWrite()中
 发送剩余的数据。如果当前outputBuffer_已经有待发送的数据，那么就不能先尝试发送了，因为这会造成数据乱序。
 */

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

void TcpConnection::sendInLoop(const muduo::string& message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    //if not care  writable event now and remain nothing to send in outputBuffer_
    if (!channel_->isWriting() && outputBuffer_.readableBytes()==0)
    {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote>=0)
        {
            //写入的数据数量小于message的大小
            if (implicit_cast<size_t>(nwrote)<message.size())
            {
                LOG_TRACE << "I am going to write more data";
            }
        }
        else
        {
            nwrote = 0;
            if (errno!=EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
            }
        }
    }
    assert(nwrote>=0);
    {
        if (implicit_cast<size_t>(nwrote)<message.size())
        {
            //append remain data to outputBuffer_
            outputBuffer_.append(message.data()+nwrote, message.size()-nwrote);
            //care writable event
            if (!channel_->isWriting())
            {
                channel_->enableWriting();
            }
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_==kConnected)
    {
        //只关闭了写入端
        //正在关闭写入端
        setState(kDisconnecting);
        loop_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    //如果outputBuffer_中没有剩余数据需要写入到socket_
    if (!channel_->isWriting())
    {
        //关闭socket的写入端
        socket_->shutdownWrite();
    }
}



//当outputBuffer_中还有数据需要传给对方的时候，会调用　handleWrite函数

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        //write data from outputBuffer_ to client
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n>0)
        {
            outputBuffer_.retrieve(n);
            //if write complete
            if (outputBuffer_.readableBytes()==0)
            {
                //diable writing now
                channel_->disableWriting();
                //如果当前状态为正在关闭连接，调用shutdownInLoop函数
                if (state_==kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
            else{
                LOG_TRACE<<"I am going to write more data!";
            }
        }
        else
        {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    }  //连接已经关闭，不能再写入了
    else
    {
        LOG_TRACE << "Connection is down, no more writing";
    }
}

