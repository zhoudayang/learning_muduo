//
// Created by zhouyang on 16-8-4.
//

#include "Acceptor.h"

#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

using namespace muduo;


Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr) :
        loop_(loop),
        //create socket
        acceptSocket_(sockets::createNonBlockingOrDie()),
        //create channel
        acceptChannel_(loop, acceptSocket_.fd()),
        listening_(false)
{
    //设置地址可复用
    acceptSocket_.setReuseAddr(true);
    //绑定监听地址
    acceptSocket_.bindAddress(listenAddr);
    //设置read call back function
    acceptChannel_.setReadCallback(
            boost::bind(&Acceptor::handleRead, this)
    );
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    //peerAddr client connection information
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd > 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        }
        else {
            sockets::close(connfd);
        }
    }
}
