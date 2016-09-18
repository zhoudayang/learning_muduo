#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <boost/bind.hpp>


class EchoServer {
public:
    EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr,int maxConnection);

    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr &con);

    void onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
    int numConnected_;
    //限制最大连接数量
    const int kMaxConnections_;
};

EchoServer::EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr,int maxConnections)
        : server_(loop, listenAddr, "EchoServer"),kMaxConnections_(maxConnections),numConnected_(0) {
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
    if(con->connected()){
        ++numConnected_;
        //如果当前连接数量超过最大连接数量，那么关闭这个新连接
        if(numConnected_>kMaxConnections_){
            con->shutdown();
        }
    }
    else
        --numConnected_;
    //输出当前连接客户端数目
    LOG_INFO<<"current numConnected = "<<numConnected_;

}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf, muduo::Timestamp time) {
    //retrieve all message received in buffer
    muduo::string msg(buf->retrieveAllAsString());
    LOG_INFO << con->name() << " echo " << msg.size() << " bytes ,"
             << "data received at " << time.toString();
    int size  = msg.size();
    for(int i=0;i<size;i++){
        char &ch = msg[i];
        if(isupper(ch)){
            ch = tolower(ch);
        }
        else if(islower(ch)){
            ch = toupper(ch);
        }
    }
    //将处理之后的内容发送给客户端
    con->send(msg);
}
int main() {
    LOG_INFO << " pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(2016);
    //set max connection to 200
    EchoServer server(&loop, listenAddr,200);
    server.start();
    loop.loop();
    return 0;
}
