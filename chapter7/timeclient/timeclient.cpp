//
// Created by zhouyang on 16-7-23.
//

#include "timeclient.h"

TimeClient::TimeClient(EventLoop *loop, const InetAddress &serverAddr)
        : loop_(loop), client_(loop, serverAddr, "TimeClient") {
    client_.setConnectionCallback(
            boost::bind(&TimeClient::onConnection, this, _1)
    );
    client_.setMessageCallback(
            boost::bind(&TimeClient::onMessage, this, _1, _2, _3)
    );
}

void TimeClient::connect() {
    client_.connect();
}

void TimeClient::onConnection(const TcpConnectionPtr &con) {
    LOG_INFO << con->localAddress().toIpPort()
             << " -> "
             << con->peerAddress().toIpPort()
             << " is "
             << (con->connected() ? "up" : "down");
    if (!con->connected()) {
        loop_->quit();
    }
}

void TimeClient::onMessage(const TcpConnectionPtr &con, Buffer *buf, Timestamp receiveTime) {
    if (buf->readableBytes() >= sizeof(int32_t)) {
        const void *data = buf->peek();
        int32_t be32 = *static_cast<const int32_t *> (data);
        buf->retrieve(sizeof(int32_t));
        time_t time = sockets::networkToHost32(be32);
        Timestamp ts(implicit_cast<uint64_t>(time) * Timestamp::kMicroSecondsPerSecond);
        LOG_INFO << "Server time = "
                 << time << ", "
                 << ts.toFormattedString();
    } else {
        LOG_INFO << con->name()
                 << " no enough data "
                 << buf->readableBytes()
                 << " at "
                 << receiveTime.toFormattedString();
    }
}