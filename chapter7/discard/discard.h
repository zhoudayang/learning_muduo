

#ifndef DISCARD_DISCARD_H
#define DISCARD_DISCARD_H

#include<muduo/net/TcpServer.h>

class DiscardServer {
public:
    DiscardServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr & con);

    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};

#endif //DISCARD_DISCARD_H
