#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>
#include <stdio.h>


class ChargenServer {
public:

    ChargenServer(muduo::net::EventLoop * loop,const muduo::net::InetAddress &listenAddress, bool print = false);

    void start();
private:


    void onConnection(const muduo::net::TcpConnectionPtr & conn);

    void onMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer * buf,muduo::Timestamp time);

    void onWriteComplete( const muduo::net::TcpConnectionPtr &conn);

    void printThroughput();

    muduo::net::TcpServer server_;

    muduo::string message_;
    int64_t transferred_;
    muduo::Timestamp startTime_;
};

using namespace muduo;
using namespace muduo::net;

ChargenServer ::ChargenServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddress, bool print)
:server_(loop,listenAddress,"ChargenServer"),
 transferred_(0),
 startTime_(Timestamp::now())
{
    server_.setConnectionCallback(boost::bind(&ChargenServer::onConnection,this,_1));
    server_.setMessageCallback(boost::bind(&ChargenServer::onMessage,this,_1,_2,_3));
    server_.setWriteCompleteCallback(boost::bind(&ChargenServer::onWriteComplete,this,_1));
    //bind function that run every 3 seconds
    if(print){
        loop->runEvery(3.0,boost::bind(&ChargenServer::printThroughput,this));
    }
    string line;
    for(int i=33;i<127;i++){
        line.push_back(char(i));
    }
    line += line;
    for(size_t i=0;i<127-33;i++){
        message_ += line.substr(i,72);
        message_+= '\n';
    }

}

void ChargenServer::start(){
    server_.start();
}

void ChargenServer::onMessage(const TcpConnectionPtr & conn,Buffer *buf,Timestamp time){
    string msg(buf->retrieveAllAsString());
    LOG_INFO<<conn->name()<<" discards " <<msg.size()<<" bytes received at " <<time.toString();
}

//call this function every 3 seconds
void ChargenServer::printThroughput() {
    Timestamp endTime = Timestamp::now();
    double time = timeDifference(endTime,startTime_);
    printf("%4.3f MiB /s \n",static_cast<double>(transferred_/time/1024/1024));
    //reset transferred_ and startTime_
    transferred_ =0;
    startTime_ = endTime;
}

//function call when build connection
void ChargenServer::onConnection(const muduo::net::TcpConnectionPtr &conn) {
    LOG_INFO<<"ChargenServer - "<< conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<" is "<< (conn->connected()?"up":"down");
    if(conn->connected()){
        conn->setTcpNoDelay(true);
        //begin to send first message
        conn->send(message_);
    }
}

//when write complete, begin to transfer net message
//net send loop begin
void ChargenServer::onWriteComplete(const muduo::net::TcpConnectionPtr &conn) {
    transferred_ += message_.size();
    conn->send(message_);
}



int main(){

    LOG_INFO<<" pid = "<<getpid();
    EventLoop loop;
    InetAddress listenAddr(2019);
    ChargenServer server(&loop,listenAddr,true);
    server.start();
    loop.loop();
    return 0;
}
//test function:
//nc localhost 2019
