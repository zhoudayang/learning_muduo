//
// Created by fit on 16-7-23.
//

#ifndef CHARGEN_CHARGEN_H
#define CHARGEN_CHARGEN_H

#include<muduo/net/TcpServer.h>

class ChargenServer {
public:
    ChargenServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr, bool print = false);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    void onWriteComplete(const muduo::net::TcpConnectionPtr &con);

    void printThroughput();

    muduo::net::TcpServer server_;
    muduo::string message_;
    int64_t transferred_;
    muduo::Timestamp startTime_;
};


#endif //CHARGEN_CHARGEN_H
