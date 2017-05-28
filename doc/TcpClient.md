# TcpClient 
TcpClient是muduo网络库中提供的用于创建tcp连接的类。 TcpClient的主要操作主要是借助Connector来实现的。

#### 连接建立
创建新连接操作：
```
void TcpClient::newConnection(int sockfd)
{
  loop_->assertInLoopThread();
  InetAddress peerAddr(sockets::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(loop_,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));

  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
  {
    MutexLockGuard lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}
```

连接建立成功之后，会创建TcpConnectionPtr，并且做好记录。在close回调之中，TcpClient首先移除了连接，然后调用了connectDestroyed方法来将连接对应的channel从poller之中移除。若设定为需要重试，那么removeConnection还会重启Connector，重新进行连接。
```
void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    MutexLockGuard lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_)
  {
    LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->serverAddress().toIpPort();
    connector_->restart();
  }
}
```

若要关闭连接，则首先判断TcpClient的TcpConnectionPtr是否有效，有效就关闭连接，并且设置connect_为false，TcpClient不再进行重试。因为可能会涉及到跨线程调用，所以在修改TcpConnectionPtr的时候会使用锁进行保护，确保线程安全。

#### TcpClient的析构函数
首先列出一下TcpClient对象的析构函数的代码：
```
TcpClient::~TcpClient()
{
  LOG_INFO << "TcpClient::~TcpClient[" << name_
           << "] - connector " << get_pointer(connector_);
  TcpConnectionPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(mutex_);
    unique = connection_.unique();
    conn = connection_;
  }
  if (conn)
  {
    assert(loop_ == conn->getLoop());
    // FIXME: not 100% safe, if we are in different thread
    CloseCallback cb = std::bind(&detail::removeConnection, loop_, _1);
    loop_->runInLoop(
        std::bind(&TcpConnection::setCloseCallback, conn, cb));
    if (unique)
    {
      conn->forceClose();
    }
  }
  else
  {
    connector_->stop();
    // FIXME: HACK
    loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
  }
}
```
可见TcpClient对象的析构函数实现比较复杂，问题的核心在于TcpConnection中的close回调调用的是TcpClient的removeConnection方法，当TcpClient对象析构之后，这一回调也就失效了。所以在析构函数中要将TcpConnection的closeCallback调整为TcpConnection中的connectDestroyed。若用户程序没有保留TcpConnectionPtr，那么就可以简单粗暴的关闭连接。否则，不能关闭连接，以保证用户程序能够继续使用。







