#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include<muduo/net/TcpClient.h>

#include<boost/bind.hpp>

#include<stdio.h>
#include<unistd.h>

using namespace muduo;
using namespace muduo::net;

class ChargenClient : boost::noncopyable {
public:
    ChargenClient(EventLoop *loop, const InetAddress &listenAddr)
            : loop_(loop), client_(loop, listenAddr, "ChargenClient") {
        client_.setConnectionCallback(
                boost::bind(&ChargenClient::onConnection, this, _1)
        );
        client_.setMessageCallback(
                boost::bind(&ChargenClient::onMessage, this, _1, _2, _3)
        );

    }

    void connect() {
        client_.connect();
    }

private:
    void onConnection(const TcpConnectionPtr &con) {
        LOG_INFO << con->localAddress().toIpPort() << " -> "
                 << con->peerAddress().toIpPort() << " is "
                 << (con->connected() ? "up" : "down");
        if (!con->connected())
            loop_->quit();
    }

    void onMessage(const TcpConnectionPtr &con, Buffer *buf, Timestamp time) {
        //重置index 不对传入的数据进行处理
        buf->retrieveAll();
    }

    EventLoop *loop_;
    TcpClient client_;
};

int main() {
    LOG_INFO << "pid= " << getpid();
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 2016);
    ChargenClient chargenClient(&loop, serverAddr);
    chargenClient.connect();
    loop.loop();


}