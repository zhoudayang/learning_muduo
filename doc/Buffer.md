#### 实现原理

在muduo之中，使用std::vector来存放网络中传输的数据，包括从要发送给对方的数据以及从对方接收的数据。在Buffer之中，预留了 prependable 的缓存，可以在buffer之中加入前缀而无需移动整个buffer。整个buffer分为prependable, readable, writable bytes. Buffer 的布局可以参考下图。

```
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode>>>
```

Buffer的作用非常之大，将用户程序从繁琐的Buffer相关读写操作之中解放了出来。


#### 重点

在muduo的Buffer之中，对read进行了特殊处理。因为从socket读，会调用read，涉及到一次上下文切换，为了减少系统调用的开销，read的调用次数是越少越好，因而需要尽可能预先分配更大的用户空间缓存。另一方面，如果对于每个连接都分配过多的缓存，那么会造成因为内存容量有限而造成支持的并发连接数目有限的问题。这二者之间存在矛盾。

在muduo之中，使用分配在堆栈上的缓存区域以及readv系统调用，将读取的数据优先存入buffer之中，超过限制才存放在堆栈上分配的缓存之中，最后再统一汇总到buffer之中。离开readFd函数之后，堆栈上分配的读取缓存会被自动回收。具体可以参考以下代码:

```
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin()+writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0)
  {
    *savedErrno = errno;
  }
  else if (implicit_cast<size_t>(n) <= writable)
  {
    writerIndex_ += n;
  }
  else
  {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  // if (n == writable + sizeof extrabuf)
  // {
  //   goto line_30;
  // }
  return n;
}
```
