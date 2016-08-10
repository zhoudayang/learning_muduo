//
// Created by zhouyang on 16-8-5.
//

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "Callbacks.h"
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

        //return loop
        EventLoop *getLoop() const {
            return loop_;
        }

        //return connection name
        const std::string &name() const {
            return name_;
        }

        //return local address
        const InetAddress &localAddr() {
            return localAddr_;
        }

        //return peer address
        const InetAddress &peerAddr() {
            return peerAddr_;
        }

        //return if connected ?
        bool connected() const {
            return state_ == kConnected;
        }

        //set connection callback function
        void setConnectionCallback(const ConnectionCallback &cb) {
            connectionCallback_ = cb;
        }

        //set message callback function
        void setMessageCallback(const MessageCallback &cb) {
            messageCallback_ = cb;
        }

        //establish connection
        void connectEstablished();

    private:
        enum StateE {
            kConnecting, kConnected,
        };

        void setState(StateE s) {
            state_ = s;
        }

        void handleRead();

        EventLoop *loop_;
        std::string name_;
        StateE state_;

        boost::scoped_ptr<Socket> socket_;
        boost::scoped_ptr<Channel> channel_;
        InetAddress localAddr_;
        InetAddress peerAddr_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;

    };


}

#endif