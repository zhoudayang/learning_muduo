#include <muduo/net/TcpServer.h>
#include <muduo/base/Atomic.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int numThreads = 0;

class DiscardServer {
public:
    DiscardServer(EventLoop *loop, const InetAddress &listenAddr)
            : server_(loop, listenAddr, "DiscardServer"), oldCounter_(0), startTime_(Timestamp::now()) {
        server_.setConnectionCallback(boost::bind(&DiscardServer::onConnection, this, _1));
        server_.setMessageCallback(boost::bind(&DiscardServer::onMessage, this, _1, _2, _3));
        //通过函数setThreadNum配置为多线程服务器，one loop per thread 多线程io模型
        server_.setThreadNum(numThreads);
        loop->runEvery(3.0, boost::bind(&DiscardServer::printThroughput, this));
    }

    void start() {
        LOG_INFO << "starting " << numThreads << " threads.";
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &con) {
        LOG_TRACE << con->peerAddress().toIpPort() << " -> "
                  << con->localAddress().toIpPort() << " is "
                  << (con->connected() ? "up" : "down");
    }

    void onMessage(const TcpConnectionPtr &con, Buffer *buf, Timestamp) {
        size_t len = buf->readableBytes();
        //传递的消息长度加上len
        transferred_.add(len);
        //消息数量加1
        receivedMessages_.incrementAndGet();
        //清空buf
        buf->retrieveAll();
    }

    void printThroughput() {
        Timestamp endTime = Timestamp::now();
        int64_t newCounter = transferred_.get();
        int64_t bytes = newCounter - oldCounter_;
        //获取至今接收的消息数目，并且将receivedMessages_ 归零
        int64_t msgs = receivedMessages_.getAndSet(0);
        double time = timeDifference(endTime, startTime_);
        printf("%4.3f MiB/s %4.3f ki Msgs/s %6.2f bytes per msg\n",
               static_cast<double>(bytes) / time / 1024 / 1024,
               static_cast<double>(msgs) / time / 1024,
               static_cast<double>(bytes) / static_cast<double>(msgs)
        );
        //重新设置oldCounter和startTime_
        oldCounter_ = newCounter;
        startTime_ = endTime;
    }

    TcpServer server_;
    AtomicInt64 transferred_;
    AtomicInt64 receivedMessages_;
    int64_t oldCounter_;
    Timestamp startTime_;
};

int main() {
    LOG_INFO << "pid = " << getpid() << ", tid= " << CurrentThread::tid();
    numThreads = 4;
    EventLoop loop;
    InetAddress listenAddr(2018);
    DiscardServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}