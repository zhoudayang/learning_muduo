//
// Created by zhouyang on 16-10-9.
//

#ifndef S05_TCPSERVER_H
#define S05_TCPSERVER_H

#include "Callback.h"
#include "TcpConnection.h"

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

//小结
//Acceptor accept　new connection, and build new socket file descriptor, use this file descriptor create TcpConnection instance, and call connectEstablished function.
//if the above socket handle read event, it will call handleRead in which will call message callback function
namespace muduo {
    class Acceptor;

    class EventLoop;

    class TcpServer : boost::noncopyable {
    public:
        TcpServer(EventLoop *loop, const InetAddress &listenAddr);

        ~TcpServer();

        void start();

        void setConnectionCallback(const ConnectionCallback &cb) {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback &cb) {
            messageCallback_ = cb;
        }

    private:
        void newConnection(int sockfd, const InetAddress &peerAddr);

        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop *loop_;
        const std::string name_;
        boost::scoped_ptr<Acceptor> acceptor_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        bool started_;
        int nextConnId_;
        ConnectionMap connections_;
    };
}


#endif //S05_TCPSERVER_H
