#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <boost/bind.hpp>
#include <algorithm>

//an simple echo server implement
class EchoServer {
public:
    EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};

EchoServer::EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr)
        : server_(loop, listenAddr, "EchoServer") {
    server_.setConnectionCallback(
            boost::bind(&EchoServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
            boost::bind(&EchoServer::onMessage, this, _1, _2, _3)
    );

}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr &con) {
    LOG_INFO << "EchoServer - "
             << con->peerAddress().toIpPort()
             << " -> "
             << con->localAddress().toIpPort()
             << "is"
             << (con->connected() ? "up" : "down");

}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time) {
    muduo::string msg(buf->retrieveAllAsString());
    LOG_INFO << con->name() << " echo " << msg.size() << " bytes ,"
             << "data received at " << time.toString();
    //对client传入的内容进行大小写互换
    std::transform(msg.begin(),msg.end(),msg.end(),[](int value){
        if(isupper(value)){
            return tolower(value);
        }
        else if(islower(value)){
            return toupper(value);
        }
        return value;
    });
    //将处理之后的内容发送给客户端
    con->send(msg);
}
int main() {
    LOG_INFO << " pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(2016);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}