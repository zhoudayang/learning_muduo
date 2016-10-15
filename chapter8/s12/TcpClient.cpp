//
// Created by zhouyang on 16-10-15.
//

#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "base/Logging.h"

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;

namespace muduo {

    namespace detail {
        void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn) {

            loop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
        }

        void removeConnector(const ConnectorPtr &connector) {

        }

    }
}

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr)
        : loop_(CHECK_NOTNULL(loop)),
          connector_(new Connector(loop, serverAddr)),
          retry_(false),
          connect_(true),
          nextConnId_(1) {
    connector_->setNewConnectionCallback(boost::bind(&TcpClient::newConnection, this, _1));
    LOG_INFO << "TcpClient::TcpClient [" << this << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient() {
    LOG_INFO << "TcpClient::~TcpClient[" << this << "] - connector " << get_pointer(connector_);
    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(mutex_);
        conn = connection_;
    };
    if (conn) {
        //set removeConnection as close callback function to TcpConnection
        CloseCallback cb = boost::bind(&detail::removeConnection, loop_, _1);
        //set Close callback function to TcpConnection
        loop_->runInLoop(boost::bind(&TcpConnection::setCloseCallback, conn, cb));
    }
    else {
        connector_->stop();
        //call remove connector callback function
        loop_->runAfter(1, boost::bind(&detail::removeConnector, connector_));
    }
}

void TcpClient::connect() {
    LOG_INFO << "TcpClient::connect[" << this << "] connecting to " << connector_->serverAddress().toHostPort();
    connect_ = true;
    //start to connect
    connector_->start();
}

//disconnect the connection
void TcpClient::disconnect() {

    connect_ = false;
    {
        MutexLockGuard lock(mutex_);
        if (connection_) {
            //shutdown the connection
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    //stop connecting
    connect_ = false;
    connector_->stop();
}

//这里是题眼
void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    //when close, call removeConnection function in TcpClient
    conn->setCloseCallback(boost::bind(&TcpClient::removeConnection, this, _1));
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());
    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }
    //destroy connection
    loop_->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
    //如果需要保持连接，且需要重试，则重新发起连接
    if (retry_ & connect_) {
        LOG_INFO << "TcpClient::connect [" << this << "} - Reconnecting to "
                 << connector_->serverAddress().toHostPort();
        connector_->restart();
    }
}