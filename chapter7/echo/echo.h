//
// Created by fit on 16-7-23.
//

#ifndef ECHO_ECHO_H
#define ECHO_ECHO_H

#include <muduo/net/TcpServer.h>

class EchoServer {
public:
    EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};


#endif //ECHO_ECHO_H
