# EventLoopThreadPool 
用途：创建一个EventLoop pool, 创建给定数目的线程，每个线程之上新建一个EventLoop对象并且启动事件循环。

其关键方法是getNextLoop, 他会采用round robin方式，每次返回下一个可用的事件循环。若pool不为空，只返回pool之中的事件循环，否则返回```baseLoop_```。这一做法在于后面TcpServer之中，只使用```baseLoop_```来处理连接建立，使用pool之中的线程用来处理和客户端的io事件。

```
EventLoop* EventLoopThreadPool::getNextLoop()
{
  baseLoop_->assertInLoopThread();
  assert(started_);
  EventLoop* loop = baseLoop_;

  if (!loops_.empty())
  {
    // round-robin
    loop = loops_[next_];
    ++next_;
    if (implicit_cast<size_t>(next_) >= loops_.size())
    {
      next_ = 0;
    }
  }
  return loop;
}
```