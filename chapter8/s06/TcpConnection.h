//
// Created by fit on 16-10-9.
//

#ifndef S05_TCPCONNECTION_H
#define S05_TCPCONNECTION_H

#include "Callback.h"
#include "InetAddress.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo {
    class Channel;

    class EventLoop;

    class Socket;

    class TcpConnection : boost::noncopyable, public boost::enable_shared_from_this<TcpConnection> {
    public:

        TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                      const InetAddress &peerAddr);


        ~TcpConnection();


        EventLoop *getLoop() const {
            return loop_;
        }

        const std::string &name() const {
            return name_;
        }

        const InetAddress &localAddress() {
            return localAddr_;
        }

        const InetAddress &peerAddress() {
            return peerAddr_;
        }

        bool connected() const {
            return state_ == kConnected;
        }

        void setConnectionCallback(const ConnectionCallback &cb) {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback &cb) {
            messageCallback_ = cb;
        }

        void setCloseCallback(const CloseCallback &cb) {
            closeCallback_ = cb;
        }

        //called when TcpServer accepts a new connection
        void connectEstablished();
        //should be called only once

        //called when TcpServer has removed me from its map
        //should be called only once
        void connectDestroyed();

    private:
        enum StateE {
            kConnecting, kConnected, kDisconnected,
        };

        void setState(StateE s) {
            state_ = s;

        }

        void handleRead();

        void handleWrite();

        void handleError();

        void handleClose();


        EventLoop *loop_;
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
    };
}


#endif //S05_TCPCONNECTION_H
