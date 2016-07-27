//
// Created by fit on 16-7-27.
//

#ifndef NEW_PROJECT_ECHO_H
#define NEW_PROJECT_ECHO_H

#include <muduo/net/TcpServer.h>
#include <boost/circular_buffer.hpp>
#include <boost/version.hpp>
#include <boost/unordered_set.hpp>

#if BOOST_VERSION < 104700

namespace boost{
    template<typename T>
    inline size_t hash_value(const boost::shared_ptr<T>&x){
        return boost::hash_value(x.get());
    }
}

#endif

//RFC 862
class EchoServer {
public:
    EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr, int idleSeconds);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    void onTimer();

    void dumpconnectionBuckets() const;

    //存放TcpConnection的　weakPtr指针
    typedef boost::weak_ptr<muduo::net::TcpConnection> WeakTcpconnectionPtr;

    struct Entry {
        explicit Entry(const WeakTcpconnectionPtr &weakConn) : weakConn_(weakConn) {}

        ~Entry() {
            muduo::net::TcpConnectionPtr con = weakConn_.lock();
            if (con)
                con->shutdown();
        }

        WeakTcpconnectionPtr weakConn_;
    };

    typedef boost::shared_ptr<Entry> EntryPtr;
    typedef boost::weak_ptr<Entry> WeakEntryPtr;
    typedef boost::unordered_set<EntryPtr> Bucket;
    typedef boost::circular_buffer<Bucket> WeakConnectionList;

    muduo::net::TcpServer server_;
    //circular buffer 实际上是基于vector实现的具有固定大小的循环缓冲块，
    //伴随着插入会自动释放buffer头部的对象，整体前移以便有空间存储插入的新对象
    WeakConnectionList connectionBuckets_;
};


#endif //NEW_PROJECT_ECHO_H
