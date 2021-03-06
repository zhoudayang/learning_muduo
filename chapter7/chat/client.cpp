#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include<iostream>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

class ChatClient : boost::noncopyable {
public:
    ChatClient(EventLoop *loop, const InetAddress &serverAddr)
            : client_(loop, serverAddr, "ChatClient"),
              codec_(boost::bind(&ChatClient::onStringMessage, this, _1, _2, _3)) {
        client_.setConnectionCallback(boost::bind(&ChatClient::onConnection, this, _1));
        //bind message callback function to LengthHeaderCodec onMessage
        client_.setMessageCallback(boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
        //关闭nagle算法，保证数据包立即发送，避免产生延时
        client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

    //write 会由main线程来调用，所以需要加锁来保护shared_ptr
    void write(const StringPiece &message) {
        //这个锁是为了保护shared_ptr
        MutexLockGuard lock(mutex_);
        //if connection_ is valid, send message to server
        if (connection_) {
            codec_.send(get_pointer(connection_), message);
        }
    }

private:
    //onConnetion 会被EventLoop 线程调用，所以要加锁保护shared_ptr
    void onConnection(const TcpConnectionPtr &con) {
        LOG_INFO << con->localAddress().toIpPort() << " -> "
                 << con->peerAddress().toIpPort() << " is "
                 << (con->connected() ? "UP" : "DOWN");
        MutexLockGuard lock(mutex_);
        if (con->connected())
            connection_ = con;
            //if server close the connection, reset connection_
        else
            connection_.reset();
    }

    //输出服务器发送的消息
    void onStringMessage(const TcpConnectionPtr &con, const string &message, Timestamp time) {
        printf("<<< %s\n", message.c_str());
    }

    TcpClient client_;
    LengthHeaderCodec codec_;
    MutexLock mutex_;
    TcpConnectionPtr connection_;
};

int main() {
    LOG_INFO << " pid= " << getpid();
    EventLoopThread loopThread;
    uint16_t port = 2016;
    //服务器套接字地址
    InetAddress serverAddr("127.0.0.1", port);
    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line)) {
        //call write function in main thread
        client.write(line);
    }
    client.disconnect();
    //等待连接完全断开
    CurrentThread::sleepUsec(1000 * 1000);
}