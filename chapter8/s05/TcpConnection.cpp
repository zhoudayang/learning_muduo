//
// Created by fit on 16-10-9.
//

#include "TcpConnection.h"

#include "base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Sockets.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : loop_(CHECK_NOTNULL(loop)),
          name_(name),
          state_(kConnecting),
          socket_(new Socket(sockfd)),
          channel_(new Channel(loop, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr) {
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at"
              << this << " fd=" << sockfd;
    //set read callback function for channel of this TcpConnection
     channel_->setReadCallback(
      boost::bind(&TcpConnection::handleRead, this));

}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at" << this << " fd=" << channel_->fd();
}


void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_==kConnecting);
    setState(kConnected);
    channel_->enableReading();
    //call connection callback function
    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead() {
    char buf[65536];
    ssize_t n = ::read(channel_->fd(), buf, sizeof buf);
    //call message callback function
    //typedef boost::function<void(const TcpConnectionPtr &,const char * data,ssize_t len)> MessageCallback;
    messageCallback_(shared_from_this(),buf,n);
}

