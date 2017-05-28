# TcpServer 
#### TcpServer简介
在tcp网络编程之中，服务器比客户端更容易实现。对应TcpServer的实现，总共不过200多行代码。

核心数据成员：
```
typedef std::map<string, TcpConnectionPtr> ConnectionMap;
ConnectionMap connections_;
```

TcpServer的使用流程如下所示：
1. 创建TcpServer对象
2. 设置各种回调函数，包括MessageCallback以及ConnectionCallback
3. 设置io的线程数目
4. 开启监听

#### 创建连接
当Acceptor成功接受了一个连接之后，会调用newConnection函数。

```
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
  loop_->assertInLoopThread();
  EventLoop* ioLoop = threadPool_->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  LOG_INFO << "TcpServer::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peerAddr.toIpPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}
```
从上述代码可见，TcpServer为新的连接对应的sockfd创建了TcpConnection对象，并且设置了各种回调。重点操作是对于负载均衡的处理，TcpServer从EventLoop池中取出一个loop用于处理新的连接对应的io时间，实现了多线程负载均衡服务。在muduo的TcpServer之中，使用专一的loop用来处理接受连接，使用EventLoop池中的loop用来处理与客户端之间的io事件。

#### 移除连接
在muduo的TcpServer之中，移除连接需要从对应的io线程之中移除连接，而非接受连接的那个线程。所以，对于removeConnection以及TcpServer的析构函数需要进行特殊处理，将remove操作锁定在连接对应的io线程之中运行。

```
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
}
```

```
TcpServer::~TcpServer()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (ConnectionMap::iterator it(connections_.begin());
      it != connections_.end(); ++it)
  {
    TcpConnectionPtr conn = it->second;
    it->second.reset();
    conn->getLoop()->runInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
    conn.reset();
  }
}

```





