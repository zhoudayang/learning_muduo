//
// Created by zhouyang on 16-8-4.
//
#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <stdio.h>

void newConnection(int sockfd,const muduo::InetAddress &peerAddr){
    printf("new Connection() accpted a new connection from %s \n",peerAddr.toHostPort().c_str());
    ::write(sockfd,"how are you?\n",13);
    muduo::sockets::close(sockfd);

}
/*
 * POLLNVAL means that the file descriptor value is invalid.
 * It usually indicates an error in your program,
 * but you can rely on poll returning POLLNVAL
 * if you've closed a file descriptor and you haven't opened any file since then that might have reused the descriptor.
*/
 int main(){
    printf("main(): pid= %d\n",getpid());
    muduo::InetAddress listenAddr(9987);
    muduo::EventLoop loop;
    muduo::Acceptor acceptor(&loop,listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();
    loop.loop();
}