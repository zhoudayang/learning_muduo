#include "codec.h"
#include "dispatcher.h"
#include "query.pb.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>


using namespace muduo;
using namespace muduo::net;

typedef boost::shared_ptr<muduo::Query> QueryPtr;
typedef boost::shared_ptr<muduo::Answer> AnswerPtr;

class QueryServer:boost::noncopyable{
public:
    QueryServer(EventLoop *loop,const InetAddress &listenAddr):
            server_(loop,listenAddr,"QueryServer"),
            dispatcher_(boost::bind(&QueryServer::onUnknownMessage,this,_1,_2,_3)),
            codec_(boost::bind(&ProtobufDispatcher::onProtobufMessage,&dispatcher_,_1,_2,_3))
    {
        dispatcher_.registerMessageCallback<muduo::Query>(
                boost::bind(&QueryServer::onQuery,this,_1,_2,_3)
        );
        dispatcher_.registerMessageCallback<muduo::Answer>(
                boost::bind(&QueryServer::onAnswer,this,_1,_2,_3)
        );
        server_.setConnectionCallback(
                boost::bind(&QueryServer::onConnection,this,_1)
        );
        server_.setMessageCallback(
                boost::bind(&ProtobufCodec::onMessage,&codec_,_1,_2,_3)
        );
    }
    //start server
    void start(){
        server_.start();
    }
private:
    //call this callback function when connection status changed
    void onConnection(const TcpConnectionPtr & conn){
        LOG_INFO<<conn->localAddress().toIpPort()<<" -> "<<conn->peerAddress().toIpPort()<<" is "<<(conn->connected()? "up":"down");
    }
    //call this function when meet unknown message
    void onUnknownMessage(const TcpConnectionPtr &conn,const MessagePtr &message,Timestamp){
        LOG_INFO<<"onUnknownMessage: "<<message->GetTypeName();
        //未知消息，直接关闭连接
        conn->shutdown();
    }

    void onQuery(const muduo::net::TcpConnectionPtr &conn,const QueryPtr & message,muduo::Timestamp){
        LOG_INFO<<"onQuery:\n"<<message->GetTypeName()<<message->DebugString();
        Answer answer;
        answer.set_id(1);
        answer.set_questioner("zhouyang");
        answer.set_answerer("www.cnblogs.com/zhoudayang");
        answer.add_solution("jump");
        answer.add_solution("win!");
        codec_.send(conn,answer);
        //answer the client and shutdown the connection
        conn->shutdown();
    }

    void onAnswer(const muduo::net::TcpConnectionPtr &conn,const AnswerPtr &message,muduo::Timestamp){
        LOG_INFO<<"onAnswer: "<<message->GetTypeName();
        conn->shutdown();
    }
    TcpServer server_;
    ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
};


int main(int argc,char ** argv){
    LOG_INFO<<"pid = "<<getpid();
    if(argc>1){
        EventLoop loop;
        uint16_t port = static_cast<int16_t> (atoi(argv[1]));
        InetAddress serverAddr(port);
        QueryServer server(&loop,serverAddr);
        server.start();
        loop.loop();
    }
    else{
        printf("Usage: %s port \n",argv[0]);
    }
    return 0;
}
