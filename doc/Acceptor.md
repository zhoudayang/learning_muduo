#### 运行流程
Acceptr是muduo之中TcpServer用于接受连接的wrapper类。其中有一个成员acceptChannel_用于接受连接。其句柄是用于接受连接的socket fd。

上述acceptChannel_关注的是可读事件，可读代表有新的连接可以accept了。在关注可读事件之前需要首先开始监听。监听的backlog的限制采用的是Linux所支持的最大值: SOMAXCONN.

#### 特殊处理
在接受连接的时候，可能会出现这种问题：描述符用完了，这会导致accpet失败，并且返回*ENFILE*错误。但是并没有拒绝这一连接，连接仍然会在连接队列之中，这导致了下一次eventLoop中仍然会触发监听描述符的可读事件，这会导致busy loop。

* 一种比较简单的解决方式是程序遇到这个问题，直接忽略，直到这种情况消失，但是这种解决方式会导致busy waiting。
* 另一种解决思路是记录除了EAGAIN或者EWOULDBLOCK其他任何错误，告诉用户出现了某种错误，并且停止监听描述符的可读事件，减少CPU的使用。
* 在libevent中，采用的是如下解决方式。首先```open /dev/null```, 保留一个文件描述符，当accept出现ENFILE或者EMFILE错误的时候，```close /dev/null```,然后再次accept，并且close掉accept产生的fd，再次```open /dev/null```，这是一种比较优雅的方式来拒绝客户端的连接。
* 最后一种比较sb的方式是遇到accept的这种错误，直接拒绝并且推出。这种方式比较容易受到Dos攻击。


```
void Acceptor::handleRead()
{
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  //FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0)
  {
    // string hostport = peerAddr.toIpPort();
    // LOG_TRACE << "Accepts of " << hostport;
    if (newConnectionCallback_)
    {
      newConnectionCallback_(connfd, peerAddr);
    }
    else
    {
      sockets::close(connfd);
    }
  }
  else
  {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE)
    {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}
```