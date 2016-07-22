//
// Created by zhouyang on 16-7-23.
//

#ifndef TIMECLIENT_TIMECLIENT_H
#define TIMECLIENT_TIMECLIENT_H

#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;


class TimeClient : boost::noncopyable {
public:
    TimeClient(EventLoop *loop, const InetAddress &serverAddr);

    void connect();

private:
    muduo::net::EventLoop *loop_;
    muduo::net::TcpClient client_;

    void onConnection(const TcpConnectionPtr &con);

    void onMessage(const TcpConnectionPtr &con, Buffer *buf, Timestamp receiveTime);
};


#endif //TIMECLIENT_TIMECLIENT_H
