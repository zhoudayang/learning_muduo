//
// Created by fit on 16-7-23.
//

#include "chargen.h"

#include <muduo/base/Logging.h>
#include<muduo/net/EventLoop.h>

#include<boost/bind.hpp>
#include<stdio.h>

using namespace muduo;
using namespace muduo::net;

ChargenServer::ChargenServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr, bool print)
        : server_(loop, listenAddr, "ChargenServer"), transferred_(0), startTime_(Timestamp::now()) {
    server_.setConnectionCallback(
            boost::bind(&ChargenServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
            boost::bind(&ChargenServer::onMessage, this, _1, _2, _3)
    );
    server_.setWriteCompleteCallback(
            boost::bind(&ChargenServer::onWriteComplete, this, _1)
    );
    if (print) {
        // 每３秒执行一次输出
        loop->runEvery(3.0, boost::bind(&ChargenServer::printThroughput, this));
    }
    string line;
    for (int i = 33; i < 127; i++)
        line.push_back(char(i));
    line += line;
    for (size_t i = 0; i < 127 - 33; ++i)
        message_ += (line.substr(i, 72)+'\n' );
}

void ChargenServer::start() {

    server_.start();
}

void ChargenServer::onConnection(const muduo::net::TcpConnectionPtr &con) {
    LOG_INFO << "ChargenServer - " << con->peerAddress().toIpPort()
             << con->localAddress().toIpPort() << " is "
             << (con->connected() ? "up" : "down");
    if (con->connected()) {
        con->setTcpNoDelay(true);
        con->send(message_);
    }
}

void ChargenServer::onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf,
                              muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << con->name() << " discards " << msg.size()
             << " bytes received at" << time.toString();
}

// 写入完成，执行下一次发送
void ChargenServer::onWriteComplete(const muduo::net::TcpConnectionPtr &con) {
    transferred_ += message_.size();
    con->send(message_);
}

void ChargenServer::printThroughput() {
    Timestamp endTime = Timestamp::now();
    double time = timeDifference(endTime, startTime_);
//    计算数据传输速度
    printf("%4.3f MiB/s\n", static_cast<double> (transferred_) / time / 1024 / 1024);
    startTime_ = endTime;
}