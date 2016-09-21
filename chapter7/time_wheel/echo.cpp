//
// Created by zhouyang on 16-9-21.
//

#include "echo.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

EchoServer::EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr, int idleSeconds) :
    server_(loop,listenAddr,"EchoServer"),
    connectionBuckets_(idleSeconds)
{
    //set connection callback function
    server_.setConnectionCallback(boost::bind(&EchoServer::onConnection,this,_1));
    //set message callback function
    server_.setMessageCallback(boost::bind(&EchoServer::onMessage,this,_1,_2,_3));
    //run onTimer function every 1.0 second
    loop->runEvery(1.0,boost::bind(&EchoServer::onTimer,this));
    //初始化idleSeconds个Bucket
    connectionBuckets_.resize(idleSeconds);
    //输出最新的connectionBuckets_的情况
    dumpConnectionBuckets();
}

void EchoServer::start() {
    server_.start();
}
void EchoServer::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO<<"EchoServer - "<<conn->peerAddress().toIpPort() << " -> "<<conn->localAddress().toIpPort()<<" is "<< (conn->connected()? "up":"down");
    if(conn->connected()){
        EntryPtr entry(new Entry(conn));
        //向connectionBuckets_的末尾插入entry
        connectionBuckets_.back().insert(entry);
        dumpConnectionBuckets();
        WeakEntryPtr weakEntryPtr (entry);
        //set weakEntryPtr as conntext of conn
        conn->setContext(weakEntryPtr);
    }
        //for client close the connection
    else{
        assert(!conn->getContext().empty());
        WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
        LOG_DEBUG<<"Entry use_count = "<<weakEntry.use_count();
    }
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO<<conn->name()<<" echo "<<msg.size()<<" bytes at "<<time.toString();
    LOG_DEBUG<<msg;
    conn->send(msg);
    assert(!conn->getContext().empty());
    WeakEntryPtr weakEntry = boost::any_cast<WeakEntryPtr>(conn->getContext());
    EntryPtr entry(weakEntry.lock());
    if(entry){
        //将entry插入connectionBuckets_末尾的Bucket中
        connectionBuckets_.back().insert(entry);
        dumpConnectionBuckets();
    }
}

void EchoServer::onTimer() {
    connectionBuckets_.push_back(Bucket());
    dumpConnectionBuckets();
}

void EchoServer::dumpConnectionBuckets() const {
    LOG_INFO<<"size = "<<connectionBuckets_.size();
    int idx = 0;
    for(WeakConnectionList::const_iterator it = connectionBuckets_.begin();it!= connectionBuckets_.end();it++,idx++){
        const Bucket &bucket = * it;
        printf("[%d] len = %zd :",idx,bucket.size());
        for(Bucket::const_iterator i = bucket.begin();i!= bucket.end();i++){
            bool connectionDead = (*i)->weakConn_.expired();
            //get_pointer get pointer address
            printf("%p(%ld)%s ",boost::get_pointer(*i),i->use_count(),connectionDead? "Dead": "");
        }
    }
    puts("");
}














