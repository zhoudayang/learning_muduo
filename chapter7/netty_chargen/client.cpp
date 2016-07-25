
#include <muduo/net/TcpClient.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class DiscardClient : boost::noncopyable {
public:
    DiscardClient(EventLoop *loop, const InetAddress &listenAddr, int size)
            : loop_(loop), client_(loop, listenAddr, "DiscardClient") ,message_(size,'H'){
        client_.setConnectionCallback(
                boost::bind(&DiscardClient::onConnection, this, _1)
        );
        client_.setMessageCallback(
                boost::bind(&DiscardClient::onMessage, this, _1, _2, _3)
        );
        client_.setWriteCompleteCallback(
                boost::bind(&DiscardClient::onWriteComplete, this, _1)
        );
        client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

private:
    void onConnection(const TcpConnectionPtr &con) {
        LOG_TRACE << con->localAddress().toIpPort()
                  << " -> " << con->localAddress().toIpPort()
                  << " is " << (con->connected() ? "up" : "down");
        if (con->connected()) {
            con->setTcpNoDelay(true);
            con->send(message_);
        }
        else
            loop_->quit();
    }

    void onMessage(const TcpConnectionPtr &con, Buffer *buf, Timestamp time) {
        buf->retrieveAll();
    }
    //when write complete , send next message
    void onWriteComplete(const TcpConnectionPtr &con) {
        LOG_INFO << "write complete " << message_.size();
        con->send(message_);
    }

    EventLoop *loop_;
    TcpClient client_;
    string message_;
};

int main() {
    LOG_INFO << " pid = " << getpid() << ",tid= " << CurrentThread::tid();
    EventLoop loop;
    InetAddress serverAddr("localhost", 2018);
    //在本机上运行的时候，消息的长度和服务器的吞吐量大致为线性关系，服务器最大吞吐量可以达到1400mb/s左右，
    //如果在局域网上运行，受限于局域网带宽，最高只能达到10mb/s
    int size = 65536;
    DiscardClient client(&loop, serverAddr, size);
    client.connect();
    loop.loop();
}