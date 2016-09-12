#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <set>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer : boost::noncopyable {
public:
    ChatServer(EventLoop *loop, const InetAddress &listenAddr)
            : server_(loop, listenAddr, "ChatServer"),
              codec_(boost::bind(&ChatServer::onStringMessage, this, _1, _2, _3)) {
        server_.setConnectionCallback(boost::bind(&ChatServer::onConnection, this, _1));
        //发送消息时，会调用LengthHeaderCodec中的消息发送函数，自动进行消息的打包处理
        server_.setMessageCallback((boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3)));

    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &con) {
        LOG_INFO << con->localAddress().toIpPort() << " -> "
                 << con->peerAddress().toIpPort() << " is "
                 << (con->connected() ? "up" : "down");
        //if client build the connection, add TcpConnectionPtr to set, else if client close the connection, erase it from the set
        if (con->connected()) {
            connections_.insert(con);
        }
        //将已经断开的连接从容器中删除，避免了内存和资源的泄露
        else
            connections_.erase(con);
    }

    void onStringMessage(const TcpConnectionPtr &con, const string &message, Timestamp time) {
        //向所有的客户端发送消息
        for (ConnectionList::iterator it = connections_.begin(); it != connections_.end(); it++) {
            //此处调用了消息处理中间件中的消息发送函数
            codec_.send(get_pointer(*it), message);
        }
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;
    TcpServer server_;
    //消息处理中间件
    LengthHeaderCodec codec_;
    //存放所有建立的连接
    ConnectionList connections_;
};

int main() {
    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    uint16_t port = 2016;
    InetAddress serverAddr(port);
    ChatServer server(&loop, serverAddr);
    server.start();
    loop.loop();
}