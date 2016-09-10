#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include<muduo/net/Endian.h>
#include<boost/bind.hpp>
#include <muduo/net/TcpServer.h>

class TimeServer {
public:
    TimeServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    void start();

private:
    //connection callback function
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    //message callback function
    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};

using namespace muduo;

using namespace muduo::net;

TimeServer::TimeServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr)
        : server_(loop, listenAddr, "TimeServer") {
    //set connection call
    server_.setConnectionCallback(
            boost::bind(&TimeServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
            boost::bind(&TimeServer::onMessage, this, _1, _2, _3)
    );
}

void TimeServer::start() {
    server_.start();
}

void TimeServer::onConnection(const muduo::net::TcpConnectionPtr &con) {
    LOG_INFO << "TimeServer - "
             << con->peerAddress().toIpPort()
             << " -> "
             << con->localAddress().toIpPort()
             << " is "
             << (con->connected() ? "up" : "down");
    if (con->connected()) {
        time_t now = ::time(NULL);
        int32_t be32 = sockets::hostToNetwork32(static_cast<int32_t >(now));
        con->send(&be32, sizeof(be32));
        //close connection after send the time data
        con->shutdown();
    }
}

void TimeServer::onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << con->name()
             << " discards "
             << msg.size()
             << " bytes received at "
             << time.toString();
}

int main() {

    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    InetAddress listenAddr(2016);
    TimeServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}
