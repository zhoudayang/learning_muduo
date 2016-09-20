#include "dispatcher.h"
#include "codec.h"
#include "query.pb.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

typedef boost::shared_ptr<muduo::Query> QueryPtr;
typedef boost::shared_ptr<muduo::Answer> AnswerPtr;
typedef boost::shared_ptr<muduo::Empty> EmptyPtr;

google::protobuf::Message * messageToSend;

class QueryClient:boost::noncopyable{
public:
    QueryClient(EventLoop * loop,const InetAddress &serverAddr):
        loop_(loop),
        client_(loop,serverAddr,"QueryClient"),
        dispatcher_(boost::bind(&QueryClient::onUnknownMessage,this,_1,_2,_3)),
        codec_(boost::bind(&ProtobufDispatcher::onProtobufMessage,&dispatcher_,_1,_2,_3))
    {
        dispatcher_.registerMessageCallback<muduo::Answer>(boost::bind(&QueryClient::onAnswer,this,_1,_2,_3));
        dispatcher_.registerMessageCallback<muduo::Empty>(boost::bind(&QueryClient::onEmpty,this,_1,_2,_3));
        client_.setConnectionCallback(boost::bind(&QueryClient::onConnection,this,_1));
        //message callback funtion -> ProtobufCodec onMessage
        client_.setMessageCallback(boost::bind(&ProtobufCodec::onMessage,&codec_,_1,_2,_3));
    }

    void connect(){
        client_.connect();
    }
private:
    void onConnection(const TcpConnectionPtr & conn){
        LOG_INFO<< conn->localAddress().toIpPort()<<" -> "
            <<conn->peerAddress().toIpPort()<<" is "
            <<(conn->connected()?"up":"down");
        if(conn->connected()){
            //send message
            codec_.send(conn,*messageToSend);
        }
            //if server close the connection, quit loop, and terminate
        else{
            loop_->quit();
        }
    }
    void onUnknownMessage(const TcpConnectionPtr &,const MessagePtr &message,Timestamp){
        LOG_INFO<<"onUnknownMessage: "<<message->GetTypeName();
    }
    void onAnswer(const TcpConnectionPtr &,const AnswerPtr &message,Timestamp){
        LOG_INFO<<"onAnswer:\n"<<message->GetTypeName()<<message->DebugString();
    }
    void onEmpty(const TcpConnectionPtr &,const EmptyPtr &message,Timestamp){
        LOG_INFO<<"onEmpty: "<<message->GetTypeName();
    }
    EventLoop * loop_;
    TcpClient client_;
    ProtobufCodec codec_;
    ProtobufDispatcher dispatcher_;
};

int main(int argc, char ** argv){
    LOG_INFO<<"pid = "<<getpid();
    if(argc>2){
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1],port);

        //默认发送Query　message
        muduo::Query query;
        query.set_id(1);
        query.set_questioner("zhouyang");
        query.add_question("Running ?");

        muduo::Empty empty;

        messageToSend = &query;
        if(argc >3 and argv[3][0]=='e'){
            messageToSend = &empty;
        }
        QueryClient client(&loop,serverAddr);
        client.connect();
        loop.loop();
    }
    else{
        printf("usage: %s host_ip port [q|e]\n",argv[0]);
    }
    return 0;
}