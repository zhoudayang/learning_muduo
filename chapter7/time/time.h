//
// Created by zhouyang on 16-7-23.
//

#ifndef TIME_TIME_H
#define TIME_TIME_H

#include <muduo/net/TcpServer.h>

class TimeServer {
public:
    TimeServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};


#endif //TIME_TIME_H
