

#ifndef DISCARD_DISCARD_H
#define DISCARD_DISCARD_H

#include<muduo/net/TcpServer.h>

class DiscardServer {
public:
    DiscardServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    //start server
    void start();

private:
    //function called when on connection
    void onConnection(const muduo::net::TcpConnectionPtr & con);
    //function called when on message
    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);
    //Tcp Server instance
    muduo::net::TcpServer server_;
};

#endif //DISCARD_DISCARD_H
