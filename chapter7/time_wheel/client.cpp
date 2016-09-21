//
// Created by zhouyang on 16-9-21.
//

#include <muduo/base/Logging.h>
#include <muduo/net/TcpClient.h>
#include <boost/noncopyable.hpp>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>
#include <muduo/base/Types.h>




class Client:boost::noncopyable{
public:
    Client(muduo::net::EventLoop * loop,const muduo::net::InetAddress & serverAddr):
        loop_(loop),
        client_(loop,serverAddr,"EchoClient")
    {
        client_.setConnectionCallback(boost::bind(&Client::onConnection,this,_1));
        client_.setMessageCallback(boost::bind(&Client::onMessage,this,_1,_2,_3));

    }
    void connect(){
        client_.connect();
    }
private:

    void onConnection(const muduo::net::TcpConnectionPtr &conn){
        LOG_INFO<< conn->localAddress().toIpPort()<<" -> "<<conn->peerAddress().toIpPort()<<" is "<<(conn->connected()?"up":"down");
        if(conn->connected()){
            LOG_INFO<<"connect to server successful";
        }
        else{
            LOG_INFO<<"retry to connect to server ";
            connect();
        }
    }

    void onMessage(const muduo::net::TcpConnectionPtr  &conn,  muduo::net::Buffer * buf,muduo::Timestamp receiveTime){
        muduo::string msg(buf->retrieveAllAsString());
        LOG_INFO<<conn->name()<<" echo "<<msg.size()<<" bytes at "<<receiveTime.toString();
        LOG_DEBUG<<msg;
    }
    muduo::net::EventLoop * loop_;
    muduo::net::TcpClient client_;
};

using namespace muduo;
using namespace muduo::net;

int main(int argc,char ** argv){
    EventLoop loop;
    InetAddress serverAddr;
    if(argc>1)
        serverAddr = InetAddress(argv[1],2007);
    else
        serverAddr = InetAddress("127.0.0.1",2007);
    Client client(&loop,serverAddr);
    client.connect();
    loop.loop();
    return 0;
}