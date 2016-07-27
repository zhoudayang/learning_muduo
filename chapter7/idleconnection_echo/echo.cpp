//
// Created by fit on 16-7-27.
//

#include "echo.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

#include <assert.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

EchoServer::EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr, int idleSeconds)
        : server_(loop, listenAddr, "EchoServer"), connectionBuckets_(idleSeconds) {
    server_.setConnectionCallback(boost::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
    loop->runEvery(1.0, boost::bind(&EchoServer::onTimer, this));
    connectionBuckets_.resize(idleSeconds);
    dumpconnectionBuckets();
}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
        EntryPtr entry(new Entry(conn));
        connectionBuckets_.back().insert(entry);
        dumpconnectionBuckets();
        WeakEntryPtr weakEntry(entry);
        conn->setContext(weakEntry);
    } else {
        assert(!conn->getContext().empty());
        WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
        LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
    }
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << con->name() << " echo " << msg.size() << " bytes at " << time.toString();
    con->send(msg);
    //断言　context 不为空
    assert(!con->getContext().empty());
    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(con->getContext()));
    EntryPtr entry(weakEntry.lock());
    if (entry) {
        connectionBuckets_.back().insert(entry);
        dumpconnectionBuckets();
    }
}

//定时器　每秒都会执行此函数
void EchoServer::onTimer() {
    //插入新的Bucket　头部的Bucket被释放
    connectionBuckets_.push_back(Bucket());
    dumpconnectionBuckets();
}

void EchoServer::dumpconnectionBuckets() const {
    LOG_INFO << " size= " << connectionBuckets_.size();
    int idx = 0;
    for (WeakConnectionList::const_iterator bucketIt = connectionBuckets_.begin();
         bucketIt != connectionBuckets_.end(); bucketIt++, idx++) {
        const Bucket &bucket = *bucketIt;
        printf("[%d] len = %zd: ", idx, bucket.size());
        for (Bucket::const_iterator it = bucket.begin(); it != bucket.end(); ++it) {
            //true if the managed object has already been deleted, false otherwise
            bool connectionDead = (*it)->weakConn_.expired();
            printf("%p(%ld)%s, ", get_pointer(*it), it->use_count(), connectionDead ? "Dead" : "");
        }
        puts("");
    }
}