//
// Created by zhouyang on 16-10-15.
//

#include "EventLoop.h"

#include "InetAddress.h"
#include "TcpClient.h"

#include "base/Logging.h"

#include <boost/bind.hpp>

#include <utility>

#include <stdio.h>
#include <unistd.h>

muduo::string message = "hello\n";

void onConnection(const muduo::TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
        conn->send(message);
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onMessage(const muduo::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp receiveTime) {
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(),
           conn->name().c_str(),
           receiveTime.toFormattedString().c_str());

    printf("onMessage(): [%s]\n", buf->retrieveAllAsString().c_str());
}

void writeComplete(const muduo::TcpConnectionPtr & conn){
    printf("[%s] writeComplete\n",conn->name().c_str());
}

//TcpClient test pass
int main()
{
    muduo::EventLoop loop;
    muduo::InetAddress serverAddr("localhost", 9981);
    muduo::TcpClient client(&loop, serverAddr);

    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.setWriteCompleteCallback(writeComplete);
    client.enableRetry();
    client.connect();
    loop.loop();
}
