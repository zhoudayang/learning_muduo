//
// Created by zhouyang on 16-8-5.
//

#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
        : loop_(loop),
          name_(listenAddr.toHostPort()),
          acceptor_(new Acceptor(loop, listenAddr)),
          started_(false),
          nextConnId_(1) {
    //set new Connection call back function to Acceptor
    acceptor_->setNewConnectionCallback(
            boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
    }
    if (!acceptor_->listening()) {
        loop_->runInLoop(boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    //name_ host:port
    std::string connName = name_+ buf;
    printf("TcpServer::newConnection [%s] - new connection [%s] from %s \n", name_.c_str(), connName.c_str(),
           peerAddr.toHostPort().c_str());
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    //create new TcpConnection
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    //store relation between conn_name and conn
    connections_[connName] = conn;
    //set callback functions
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    //estalish connection
    conn->connectEstablished();

}
