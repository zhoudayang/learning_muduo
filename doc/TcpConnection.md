#  TcpConnection
TcpConnection实现了对连接的抽象，连接建立，连接断开，发送数据，接收数据的操作也都封装在其中，其中的难点是对连接关闭的处理。

#### 连接建立

```
TcpConnection::TcpConnection(EventLoop* loop,
                             const string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : loop_(CHECK_NOTNULL(loop)),
    name_(nameArg),
    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024)
{
  channel_->setReadCallback(
      std::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(
      std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(
      std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(
      std::bind(&TcpConnection::handleError, this));
  LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
            << " fd=" << sockfd;
  socket_->setKeepAlive(true);
}
```

从代码中可知，TcpConnection也是基于Channel实现的。在构造函数中记录了loop句柄，使用Socket对象管理sockfd。对于io事件，读事件绑定的回调函数是handleRead函数，写事件绑定的回调函数是handleWrite, close事件绑定的回调函数是handleClose, error事件绑定的回调函数是handleError。同时，在构造函数的末尾开启了socket的keepalive选项。在构造函数之中设置了连接的当前状态是kConnecting，表示正在连接。

连接完成之后，可以通过调用函数connectEstablished来设定当前状态为已经连接并且调用连接所对应的回调函数。

```
void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();
  // call connection callback function to notify TcpConnectionPtr user
  connectionCallback_(shared_from_this());
}
```

为了保证Channel在执行TcpConnection中相关io事件的回调函数的过程中，不会因为TcpConnection对象在相关回调调用过程中被析构而造成为channel设定的回调函数失效，这里使用了Channel内置的tie方法记录TcpConnection的weak\_ptr，调用回调期间将weak\_ptr提升为shared\_ptr来保证上述情况不会发生。因为有这种用法，TcpConnection对象必须使用shared\_ptr进行管理。

#### 读取数据
```
void TcpConnection::handleRead(Timestamp receiveTime)
{
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0)
  {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  }
  else if (n == 0)
  {
    handleClose();
  }
  else
  {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}
```

当有可读事件发生时，调用handleRead函数。若read返回0，代表对方关闭了连接，调用handleClose函数，若read返回值大于0，使用当前时间，inputBuffer以及本对象的shared\_ptr调用message回调。

```
void TcpConnection::handleClose()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == kConnected || state_ == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(kDisconnected);
  channel_->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  // must be the last line
  closeCallback_(guardThis);
}
```
handleClose之中将当前状态设置为DisConnected, channel不再关注任何io事件，调用connection以及close回调通知TcpConnection对象的所有者，连接已经关闭了。

需要留意的是，在调用close回调的时候，可能TcpConnection的所有者就直接将TcpConnection **丢弃**，因而这里做了guard处理。

#### 写入数据
**我们一定要保证所有的io操作都需要在io线程之中完成，这样才能实现线程安全。**数据的发送主要分为两种情况，一种是当前没有数据要发送，那么首先在io线程中尝试发送，将剩余没有发送的数据存入outputBuffer，关注可写事件。

```
void TcpConnection::sendInLoop(const void* data, size_t len)
{
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == kDisconnected)
  {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  // if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
  {
    nwrote = sockets::write(channel_->fd(), data, len);
    if (nwrote >= 0)
    {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_)
      {
        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
      }
    }
    else // nwrote < 0
    {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
      {
        LOG_SYSERR << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
        {
          faultError = true;
        }
      }
    }
  }

  assert(remaining <= len);
  if (!faultError && remaining > 0)
  {
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remaining >= highWaterMark_
        && oldLen < highWaterMark_
        && highWaterMarkCallback_)
    {
      loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
    if (!channel_->isWriting())
    {
      channel_->enableWriting();
    }
  }
  }
```

请注意，这里有对高水位回调函数的调用，高水位回调函数在outputBuffer的size大于HighWaterMark\_的时候会被调用。

```
void TcpConnection::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    ssize_t n = sockets::write(channel_->fd(),
                               outputBuffer_.peek(),
                               outputBuffer_.readableBytes());
    if (n > 0)
    {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0)
      {
        channel_->disableWriting();
        if (writeCompleteCallback_)
        {
          loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
    }
    else
    {
      LOG_SYSERR << "TcpConnection::handleWrite";
      // if (state_ == kDisconnecting)
      // {
      //   shutdownInLoop();
      // }
    }
  }
  else
  {
    LOG_TRACE << "Connection fd = " << channel_->fd()
              << " is down, no more writing";
  }
}
```
在handleWrite中继续发送剩余的数据，当发送完毕之后，会调用WriteComplete回调函数，通知TcpConnection所有者所有数据都已发送完毕。发送完所有数据会不再关注可写事件，否则会造成busy loop。

#### 连接关闭
关闭连接不可直接close对应的sockfd，否则积压在链路中未被对方应用程序接收的数据可能丢失。对此muduo的处理是首先shutdown write。关闭写之后，对方read就会返回0，此时对方就会关闭连接，这样本地sockfd就会read返回0，此时调用handleClose完成最终的连接关闭操作。这种主动的连接关闭策略是一种比较文雅的关闭方式，能够保证链路中的数据能够被对方完整的接收到。
```
void TcpConnection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
  {
    // we are not writing
    socket_->shutdownWrite();
  }
}
```

对应文雅的连接关闭方式，也有比较粗暴的连接关闭手段，那就是直接forceClose。forceClose的操作和对方关闭连接导致本地连接read返回0的操作其实是一致的。

```
void TcpConnection::forceCloseInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    // as if we received 0 byte in handleRead();
    handleClose();
  }
}
```

TcpConnection对象的主要使用者是TcpServer和TcpClient，当TcpServer对象被析构的时候，其所拥有的所有连接对象也应该被析构。但是没有那么简单，我们需要保证TcpConnection对象在析构的时候，其对应的Channel也从Poller之中移除了。

```
void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected)
  {
    setState(kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}
```
因而TcpServer在析构的时候，会对他所拥有的所有TcpConnection逐一调用connectDestroyed，保证所有的channel都被正确移除了。