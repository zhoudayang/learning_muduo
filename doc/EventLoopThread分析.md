# EventLoopThread分析
#### 用途解析
比较好的服务器模型是采用继续one loop in one thread的方式。并且最好设置EventLoop的数目最好不要超过cpu的核数，这样减少线程之间的对于cpu的争用。

EventLoop的用途是新建一个线程，在此线程上初始化EventLoop对象，启动事件循环，并且返回新建的事件循环。其具体实现可以类比为使用count设为1的CountDownLatch来完成。为了新建在新的线程上的事件循环采用下述方式：
```
EventLoop* EventLoopThread::startLoop()
{
  assert(!thread_.started());
  thread_.start();

  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL)
    {
      cond_.wait();
    }
  }

  return loop_;
}
```
thread在启动时会调用如下函数，新建并且启动EventLoop。创建EventLoop完成之后，会使用notify通知调用startLoop的线程，新的EventLoop已经就绪。

```
void EventLoopThread::threadFunc()
{
  EventLoop loop;

  if (callback_)
  {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();
  //assert(exiting_);
  loop_ = NULL;
}
```




