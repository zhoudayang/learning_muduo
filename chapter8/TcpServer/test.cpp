//
// Created by fit on 16-8-5.
//

#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <stdio.h>

void onConnection(const muduo::TcpConnectionPtr &conn) {
    if (conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddr().toHostPort().c_str());
    } else {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }

}

void onMessage(const muduo::TcpConnectionPtr &conn, const char *data, ssize_t len) {
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           len, conn->name().c_str());
    printf("the client says : %s\n", data);

}

//因为暂时还没有处理连接断开的问题，所以客户端主动断开连接会造成busy loop
int main() {
    printf("main() : pid = %d\n", getpid());
    muduo::InetAddress listenAddr(9988);
    muduo::EventLoop loop;
    muduo::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
}