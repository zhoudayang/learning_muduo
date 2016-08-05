//
// Created by zhouyang on 16-8-5.
//

#include "TcpConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : loop_(loop),
          name_(name), state_(kConnecting),
          socket_(new Socket(sockfd)),
          channel_(new Channel(loop, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr) {
    printf("TcpConnection::ctor[%s] at %p fd = %d\n", name_.c_str(), this, sockfd);
    channel_->setReadCallback(boost::bind(&TcpConnection::handleRead, this));
}

TcpConnection::~TcpConnection() {
    printf("TcpConnection::dtor[%s] at %p fd = %d\n", name_.c_str(), this, channel_->fd());
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();
    //called connection call back function
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead() {
    //read data
    char buf[65536];
    ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
    //call message callback function
    messageCallback_(shared_from_this(), buf, n);
}