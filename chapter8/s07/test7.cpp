//
// Created by fit on 16-10-8.
//

#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <stdio.h>

void newConnection(int sockfd,const muduo::InetAddress & peerAddr){
    printf("newConnection(): accpted a new Connection from %s \n ",peerAddr.toHostPort().c_str());
    ::write(sockfd,"How are you?\n",13);
    //close the connection between server and peer client
    muduo::sockets::close(sockfd);
}

int main(){
    printf("main() :pid = %d\n",getpid());
    muduo::InetAddress listenAddr(9981);
    muduo::EventLoop loop;

    muduo::Acceptor acceptor(&loop,listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();
}