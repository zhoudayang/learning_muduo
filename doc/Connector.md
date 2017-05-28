#### 怎么判断连接成功了？

```
void Connector::connect()
{
  int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno)
  {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      break;

    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}
```

可见，对于非阻塞io，如果connect之后的返回值为0，或者errno设置为EINTR, EINPROGRESS, EISCONN, 那么代表连接成功或者正在连接的过程之中。 如果errno为EAGIN，EADDRINUSE，EADDRNOTAVAIL，ECONNREFUSED，ENETURNREACH, 那么表示连接失败。在linux tcp编程之中，对于连接失败的情况，可以移植的解决方式是重新创建一个socket fd，再次尝试连接。对于EACCES，EPERM，EAFNOSUPPORT，EALREADY，EBADF，EFAULT，ENOTSOCK的情况，表示不可恢复的连接失败，直接停止连接，对于其他未知的情况也做此处理。

现在，怎么知道连接成功了呢？以connect返回的sockfd创建一个channel，关注其可写事件。若channel可写，并且尝试使用```getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen)```返回的errorCode为0，并且不是自连接，那么说明连接成功，可以将这个sockfd通过connectionCallback通知TcpClient。当然，如果调用的是ErrorCallback，那么close返回的fd，并且进行重试。

```
void Connector::handleWrite()
{
  LOG_TRACE << "Connector::handleWrite " << state_;

  if (state_ == kConnecting)
  {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err)
    {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = "
               << err << " " << strerror_tl(err);
      retry(sockfd);
    }
    else if (sockets::isSelfConnect(sockfd))
    {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    }
    else
    {
      setState(kConnected);
      if (connect_)
      {
        newConnectionCallback_(sockfd);
      }
      else
      {
        sockets::close(sockfd);
      }
    }
  }
  else
  {
    // what happened?
    assert(state_ == kDisconnected);
  }
}

```

```
// check if self connection
bool sockets::isSelfConnect(int sockfd)
{
  struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
  struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
  if (localaddr.sin6_family == AF_INET)
  {
    const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
    const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
    return laddr4->sin_port == raddr4->sin_port
        && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
  }
  else if (localaddr.sin6_family == AF_INET6)
  {
    return localaddr.sin6_port == peeraddr.sin6_port
        && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
  }
  else
  {
    return false;
  }
}
```

#### 怎么实现超时重连？
对于retry的处理使用了Connector的retry方法，他会设置超时回调，回调的时间间隔设置为从500ms~30s, 每次重新retry会将间隔的时间加倍，当然时间间隔不可超过30s。

```
void Connector::retry(int sockfd)
{
  sockets::close(sockfd);
  setState(kDisconnected);
  if (connect_)
  {
    LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
             << " in " << retryDelayMs_ << " milliseconds. ";
    loop_->runAfter(retryDelayMs_/1000.0,
                    std::bind(&Connector::startInLoop, shared_from_this()));
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  }
  else
  {
    LOG_DEBUG << "do not connect";
  }
}
```

#### 对于stop的处理
如果要停止连接，那么首先设置```connect_```为false，表示不需要建立连接了，这样若handleWrite之中返回了正常的sockfd，那么我也不要通过这个sockfd创建一个连接，而是直接将其close。并且因为```connect_```设置设置为false，后续进行retry的时候也不会尝试重新连接的。

紧接着的是对stopInLoop的调用，这里对正在处于连接状态的channel进行了reset处理，并且将没有确定连接的sockfd关闭。