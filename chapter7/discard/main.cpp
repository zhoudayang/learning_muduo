#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <muduo/net/EventLoop.h>

class DiscardServer{
public:
    DiscardServer(muduo::net::EventLoop * loop, const muduo::net::InetAddress &listenAddr);

    void start();
private:
    void onConnection(const muduo::net::TcpConnectionPtr & conn);

    void onMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer * buf, muduo::Timestamp time);

    muduo::net::TcpServer server_;
};

using namespace muduo;
using namespace muduo::net;

DiscardServer::DiscardServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr):
    server_(loop,listenAddr,"Discard Server")
{
    //set connection callback function
    server_.setConnectionCallback(boost::bind(&DiscardServer::onConnection,this,_1));
    //set message callback function
    server_.setMessageCallback(boost::bind(&DiscardServer::onMessage,this,_1,_2,_3));
}

//start the server
void DiscardServer::start(){
    //start discard server
    server_.start();
}

void DiscardServer::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO<<"Discard Server - "
                //peer address info
            <<conn->peerAddress().toIpPort() << " - > "
                //local address info
            <<conn->localAddress().toIpPort()
                //show the direction of the connection
            <<" is "<< (conn->connected() ? "UP" : "DOWN");
}

void DiscardServer::onMessage(const TcpConnectionPtr &conn,Buffer * buf,Timestamp time){
    //receive data from buffer
    string msg(buf->retrieveAllAsString());
    //count message size discard from Discard Server
    LOG_INFO<<conn->name() << " discards " << msg.size() << " bytes received at " << time.toString();
}

int main(){
    //print main thread pid
    LOG_INFO<< " pid = " <<getpid();
    EventLoop loop;
    InetAddress listenAddr(2009);
    DiscardServer server (&loop,listenAddr);
    //start the server
    server.start();
    //start event loop
    loop.loop();
}