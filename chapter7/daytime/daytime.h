

#ifndef DAYTIME_DAYTIME_H
#define DAYTIME_DAYTIME_H

#include <muduo/net/TcpServer.h>


class DaytimeServer {
public:
    DaytimeServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    //start the server
    void start();

private:
    //connection callback function
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    //message callback function
    void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};


#endif //DAYTIME_DAYTIME_H
