
#include "discard.h"

#include <muduo/base/Logging.h>
#include<boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

DiscardServer::DiscardServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr):server_(loop,listenAddr,"DiscardServer")
{
    server_.setConnectionCallback(
            boost::bind(&DiscardServer::onConnection,this,_1)
    );
    server_.setMessageCallback(
            boost::bind(&DiscardServer::onMessage,this,_1,_2,_3)
    );
}
void DiscardServer::start(){
    server_.start();
}

void DiscardServer::onConnection(const muduo::net::TcpConnectionPtr &con) {
    LOG_INFO<<"DiscardServer - "<<con->peerAddress().toIpPort()<<" -> "<<con->localAddress().toIpPort()<<" is "<<(con->connected() ?"UP":"Down");
}
void DiscardServer::onMessage(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buf,
                              muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO<<msg;
    LOG_INFO<<con->name()<<" discard "<<msg.size()<<" bytes received at "<<time.toString();
}
