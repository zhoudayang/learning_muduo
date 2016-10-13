//
// Created by fit on 16-10-9.
//

#ifndef S05_TCPCONNECTION_H
#define S05_TCPCONNECTION_H

#include "Callback.h"
#include "InetAddress.h"
#include "Buffer.h"
#include "base/Types.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
//使用 Buffer读取数据变动:
//做了一些变动：修改MessageCallback function　参数类型，在eventloop handleRead中加入参数返回时间.
//使用Buffer从socket　file descriptor中读取数据

namespace muduo {
    class Channel;

    class EventLoop;

    class Socket;

    class TcpConnection : boost::noncopyable, public boost::enable_shared_from_this<TcpConnection> {
    public:

        TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr,
                const InetAddress& peerAddr);

        ~TcpConnection();

        EventLoop* getLoop() const
        {
            return loop_;
        }

        const std::string& name() const
        {
            return name_;
        }

        const InetAddress& localAddress()
        {
            return localAddr_;
        }

        const InetAddress& peerAddress()
        {
            return peerAddr_;
        }

        bool connected() const
        {
            return state_==kConnected;
        }

        void setConnectionCallback(const ConnectionCallback& cb)
        {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback& cb)
        {
            messageCallback_ = cb;
        }

        void setCloseCallback(const CloseCallback& cb)
        {
            closeCallback_ = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        {
            writeCompleteCallback_ = cb;
        }

        //called when TcpServer accepts a new connection
        void connectEstablished();
        //should be called only once

        //called when TcpServer has removed me from its map
        //should be called only once
        void connectDestroyed();

        //thread safe
        void send(const muduo::string& message);

        //thread safe
        void shutdown();

        //add tcp no delay, off nagle algorithm
        void setTcpNoDelay(bool on);

    private:
        enum StateE {
            kConnecting, kConnected, kDisconnecting, kDisconnected,
        };

        void setState(StateE s)
        {
            state_ = s;

        }

        void handleRead(Timestamp receiveTime);

        void handleWrite();

        void handleError();

        void handleClose();

        void sendInLoop(const muduo::string& message);

        void shutdownInLoop();

        EventLoop* loop_;
        std::string name_;
        StateE state_;
        //wo don't expose those classes to client
        boost::scoped_ptr<Socket> socket_;
        boost::scoped_ptr<Channel> channel_;
        InetAddress localAddr_;
        InetAddress peerAddr_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        CloseCallback closeCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        //read data from this
        net::Buffer inputBuffer_;
        //write data to this
        net::Buffer outputBuffer_;
    };
}

#endif //S05_TCPCONNECTION_H
