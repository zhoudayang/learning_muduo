//
// Created by fit on 16-7-23.
//

#include "echo.h"
#include <muduo/base/Logging.h>
#include<boost/bind.hpp>

EchoServer::EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr)
        : server_(loop, listenAddr, "EchoServer") {
    server_.setConnectionCallback(
            boost::bind(&EchoServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
            boost::bind(&EchoServer::onMessage, this, _1, _2, _3)
    );

}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr &con) {
    LOG_INFO << "EchoServer - "
             << con->peerAddress().toIpPort()
             << " -> "
             << con->localAddress().toIpPort()
             << "is"
             << (con->connected() ? "up" : "down");

}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time) {
    muduo::string msg(buf->retrieveAllAsString());
    LOG_INFO << con->name() << " echo " << msg.size() << " bytes ,"
             << "data received at " << time.toString();
    //对client传入的内容进行大小写
    int diff1 = 'A' - 'a';
    int diff2 = -diff1;
    for (int i = 0; i < msg.length(); i++) {
        if (msg[i] >= 'a' and msg[i] <= 'z')
            msg[i] += diff1;
        else if(msg[i]>='A' and msg[i] <='Z')
            msg[i]+=diff2;
    }
    //将处理之后的内容发送给客户端
    con->send(msg);
}