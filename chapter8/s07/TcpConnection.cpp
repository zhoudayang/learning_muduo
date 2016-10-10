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

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : loop_(CHECK_NOTNULL(loop)),
          name_(name),
          state_(kConnecting),
          socket_(new Socket(sockfd)),
          channel_(new Channel(loop, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr) {
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at"
              << this << " fd=" << sockfd;
    //set read callback function for channel of this TcpConnection
    channel_->setReadCallback(boost::bind(&TcpConnection::handleRead, this,_1));
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at" << this << " fd=" << channel_->fd();
}


void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();
    //call connection callback function
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    // 使用inputBuffer_来读取数据
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
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


void TcpConnection::handleWrite() {

}


void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError[" << name_ << "] -SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected);
    channel_->disableAll();
    //call close callback function in TcpServer
    closeCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected);
    setState(kDisconnected);
    //这里与handleClose重复，这是因为某些情况下可以不经过handleClose 直接调用connecteDestroyed 函数
    channel_->disableAll();
    //call connection callback function, now state is Disconnected
    connectionCallback_(shared_from_this());
    //remove channel from poller
    loop_->removeChannel(get_pointer(channel_));
}







