
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include<boost/bind.hpp>
#include <muduo/net/TcpServer.h>

class DaytimeServer {
public:
    DaytimeServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    //start the server
    void start();

private:
    //connection callback function
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    //message callback function
    void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};

using namespace muduo;
using namespace muduo::net;

DaytimeServer::DaytimeServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr)
        : server_(loop, listenAddr, "DaytimeServer") {
    //set connection callback function
    server_.setConnectionCallback(
            boost::bind(&DaytimeServer::onConnection, this, _1)
    );
    //set message callback function
    server_.setMessageCallback(
            boost::bind(&DaytimeServer::onMessage, this, _1, _2, _3)
    );
}
//start daytime server
void DaytimeServer::start() {
    server_.start();
}

void DaytimeServer::onConnection(const muduo::net::TcpConnectionPtr &con) {
    LOG_INFO << "DaytimeServer - " << con->peerAddress().toIpPort() << " -> " << con->localAddress().toIpPort() <<
             " is " << (con->connected() ? "up" : "down");
    if (con->connected()) {
        con->send(Timestamp::now().toFormattedString() + "\n");
        //close the connection
        con->shutdown();
    }
}

//receive mesage from client and discard it
void DaytimeServer::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
                              muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << "the client says " << msg;
    LOG_INFO << conn->name() << " discards " << msg.size() << " bytes received at " << time.toString();
}

int main() {
    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    InetAddress listenAddr(2016);
    DaytimeServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}
