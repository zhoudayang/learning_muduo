//
// Created by zhouyang on 16-10-9.
//

#ifndef S05_TCPSERVER_H
#define S05_TCPSERVER_H

#include "Callback.h"
#include "TcpConnection.h"

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

//小结
//Acceptor accept　new connection, and build new socket file descriptor, use this file descriptor create TcpConnection instance, and call connectEstablished function.
//if the above socket handle read event, it will call handleRead in which will call message callback function
//add at 10.10 20:00
 /*
  handleRead函数中视read的返回值，若大于0调用MessageCallback function,　若为0，表示对方主动关闭了连接,调用handleClose 函数，
  若为负数，表示有错误发生,调用handleError函数。
  在handleClose函数中，首先disableAll of channel in TcpConnection，然后调用TcpServer 函数中的removeConnection函数，该函数首先
  从TcpServer中的ConnectionMap中删除当前TcpConnection 的shared_ptr,然后在io进程中注册调用connectDestroyed,通知用户connection回调函数对方断开连接，
  从poller中删除TcpConnection对应的channel

 */
namespace muduo {
    class Acceptor;

    class EventLoop;

    class EventLoopThreadPool;

    class TcpServer : boost::noncopyable {
    public:
        TcpServer(EventLoop *loop, const InetAddress &listenAddr);

        ~TcpServer();

        void start();

        void setConnectionCallback(const ConnectionCallback &cb) {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback &cb) {
            messageCallback_ = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback & cb){
            writeCompleteCallback_ = cb;
        }

        void setThreadNum(int numThreads);

    private:
        void newConnection(int sockfd, const InetAddress &peerAddr);

        void removeConnection(const TcpConnectionPtr &conn);

        void removeConnectionInLoop(const TcpConnectionPtr & con);


        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop *loop_;
        const std::string name_;
        boost::scoped_ptr<Acceptor> acceptor_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        bool started_;
        int nextConnId_;
        ConnectionMap connections_;
        //add here to support multi threads
        boost::scoped_ptr<EventLoopThreadPool> threadPool_;
    };
}


#endif //S05_TCPSERVER_H
