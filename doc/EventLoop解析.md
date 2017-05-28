#   EventLoop 解析 

#### 线程安全回调
在很多情况下，需要将函数操作转移到io线程之中进行，muduo EventLoop支持queueInLoop 和 runInLoop 调用，视调用线程情况，将回调函数暂时存放在vector之中，在时间循环的末尾统一进行调用处理。

为了保证输入的回调函数能够尽快完成，不然这些回调函数可能需要等到poller超时返回之后才能得以运行。muduo在事件循环中使用了eventFd，对于可能阻塞在Poller的情况，会向eventFd写入1，触发Poller调用返回，从而使回调函数得以执行。

当然，可能在其他线程之中调用queueInLoop函数，为了线程安全，存入新的回调函数的时候需要进行加锁处理。

#### 定时回调实现
##### 核心思想
在muduo之中，对于定时回调的实现主要代码在```TimerQueue.cc``` 以及```TimerQueue.h```之中。其核心思路是使用timerFd。timerFd用于通知事件循环最近一次的超时事件发生了，然后在对timerFd包装的channel的read回调函数之中，会对所有当前超时所对应的回调函数，一一调用。对于需要反复设置的定时器，也做好处理，保证timerFd能在合适的时间触发可读事件。
##### 新增回调函数
为了方便的拿到最近一个超时的时刻，并且应对多个回调函数对应一个超时条件的情况，在muduo之中使用set存放他所管理的时间回调函数，如下所示：

```
 typedef std::pair<Timestamp, Timer*> Entry;
 typedef std::set<Entry> TimerList;
 TimerList timers_;
```
如果要新增回调函数，首先将其插入到timers_之中，如果发现在加入新的定时回调之后，最早超时的时刻有变，那么重新设置timerFd，保证新增的回调函数能在准确的时间被调用。

##### cancel定时回调
如果当前事件循环正在调用定时回调函数，那么在此期间不可cancel定时回调，否则会引发未定义行为。我们只需要做好记录，确保要cancel的回调函数不会被重新设置。相反，若事件循环当前没有调用超时回调函数，那么直接将对应的定时回调从对应的数据结构之中删除即可。

这里的问题在于，如果在正在调用定时回调函数的时候来cancel，可能会造成cancel失败，这里的cancel并不是强保证。但是cancel之后，总能保证在后续reset处理之后不会再调用cancel的定时回调函数。

#### boost::any
有时候需要在不同线程的EventLoop之中记录一些EventLoop相关的信息，和muduo::TcpConnection一样，这里使用了boost::any.
```
void setContext(const boost::any& context)
{ context_ = context; }

const boost::any& getContext() const
{ return context_; }

boost::any* getMutableContext()
{ return &context_; }

boost::any context_;
```

#### 一些转调用
```
void updateChannel(Channel* channel);
void removeChannel(Channel* channel);
void hasChannel(Channel* channel);
// 转调用Poller之中的定义
其调用流程是Channel -> EventLoop -> Poller
```

#### loop分析
事件循环的退出很简单，一旦```quit_```为false，那么循环便无法继续。

```
void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_)
  {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    ++iteration_;
    if (Logger::logLevel() <= Logger::TRACE)
    {
      printActiveChannels();
    }
    // TODO sort channel by priority
    eventHandling_ = true;
    for (ChannelList::iterator it = activeChannels_.begin();
        it != activeChannels_.end(); ++it)
    {
      currentActiveChannel_ = *it;
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }
    currentActiveChannel_ = NULL;
    eventHandling_ = false;
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}
```

事件循环之中主要做两件事，调用poller获取当前的activeChannel, 然后调用这些channel的回调函数。接着处理之前从外面调入的为了保证线程安全需要在io线程之中调用的回调函数。此过程循环进行，直到显示调用quit退出事件循环。

