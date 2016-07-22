

#include "daytime.h"
#include<muduo/base/Logging.h>
#include<muduo/net/EventLoop.h>

#include<boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

DaytimeServer::DaytimeServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr)
        : server_(loop, listenAddr, "DaytimeServer") {
    server_.setConnectionCallback(
            boost::bind(&DaytimeServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
            boost::bind(&DaytimeServer::onMessage, this, _1, _2, _3)
    );
}

void DaytimeServer::start() {
    server_.start();
}

void DaytimeServer::onConnection(const muduo::net::TcpConnectionPtr &con) {
    LOG_INFO << "DaytimeServer - " << con->peerAddress().toIpPort() << " -> " << con->localAddress().toIpPort() <<
             " is " << (con->connected() ? "up" : "down");
    if (con->connected()) {
        con->send(Timestamp::now().toFormattedString() + "\n");
        con->shutdown();
    }
}

void DaytimeServer::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
                              muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << "the client says " << msg;
    LOG_INFO << conn->name() << " discards " << msg.size() << " bytes received at " << time.toString();
}
