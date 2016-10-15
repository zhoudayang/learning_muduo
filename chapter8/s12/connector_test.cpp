//
// Created by zhouyang on 16-10-15.
//

#include "EventLoop.h"
#include "Connector.h"

muduo::EventLoop *g_loop;

void connectCallback(int sockfd) {
    printf("connected!");
    g_loop->quit();
}
int main(){
    muduo::EventLoop loop;
    g_loop = & loop;
    muduo::InetAddress addr("127.0.0.1",9981);

    muduo::ConnectorPtr connector(new muduo::Connector(&loop,addr));
    connector->setNewConnectionCallback(connectCallback);
    connector->start();

    loop.loop();
}