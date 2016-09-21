//
// Created by zhouyang on 16-9-21.
//

#ifndef TIME_WHEEL_ECHO_H
#define TIME_WHEEL_ECHO_H
#include <muduo/net/TcpServer.h>
#include <boost/noncopyable.hpp>
#include <boost/unordered_set.hpp>
#include <boost/circular_buffer.hpp>

class EchoServer:boost::noncopyable{
public:
    EchoServer(muduo::net::EventLoop * loop,const muduo::net::InetAddress &listenAddr,int idleSeconds);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &conn);

    void onMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer *buf,muduo::Timestamp time);

    void onTimer();

    void dumpConnectionBuckets() const;

    typedef boost::weak_ptr<muduo::net::TcpConnection>weakTcpConnectionPtr;

    struct Entry:public muduo::copyable{
        explicit Entry(const weakTcpConnectionPtr & weakConn):weakConn_(weakConn){}

        ~Entry(){
            muduo::net::TcpConnectionPtr con = weakConn_.lock();
            //如果提升成功，那么将对应的连接关闭
            if(con){
                con->shutdown();
            }
        };

        weakTcpConnectionPtr weakConn_;
    };

    typedef boost::shared_ptr<Entry> EntryPtr;
    typedef boost::weak_ptr<Entry> WeakEntryPtr;
    typedef boost::unordered_set<EntryPtr> Bucket;
    typedef boost::circular_buffer<Bucket> WeakConnectionList;

    muduo::net::TcpServer server_;
    WeakConnectionList connectionBuckets_;

};




#endif //TIME_WHEEL_ECHO_H
