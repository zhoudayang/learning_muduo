//
// Created by zhouyang on 16-10-15.
//

#ifndef S12_TCPCLIENT_H
#define S12_TCPCLIENT_H

#include "base/Mutex.h"
#include "TcpConnection.h"

namespace muduo{
    class Connector;
    typedef boost::shared_ptr<Connector> ConnectorPtr;

    class TcpClient:boost::noncopyable{
    public:
        TcpClient(EventLoop * loop,const InetAddress & serverAddr);

        ~TcpClient();

        //因为下面3个函数需要满足多线程访问，所以需要加锁保护shared_ptr TcpConnectionPtr
        void connect();

        void disconnect();

        void stop();

        TcpConnectionPtr connection() const{
            MutexLockGuard lock(mutex_);
            return connection_;
        }

        bool retry() {
            return retry_;
        }

        void enableRetry(){
            retry_ = true;
        }

        void setConnectionCallback(const ConnectionCallback & cb){
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback & cb){
            messageCallback_ = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback & cb){
            writeCompleteCallback_ = cb;
        }


    private:

        void newConnection(int sockfd);

        void removeConnection(const TcpConnectionPtr & conn);

        EventLoop * loop_;
        ConnectorPtr connector_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;

        //retry when fail to connect ?
        bool retry_;
        bool connect_;

        int nextConnId_;
        mutable MutexLock mutex_;
        TcpConnectionPtr connection_;
    };





}


#endif //S12_TCPCLIENT_H
