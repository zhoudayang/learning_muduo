# EPollPoller
#### 默认使用什么io复用模型？

在Poller的接口类中定义了一个static函数，如下所示：
```
static Poller* newDefaultPoller(EventLoop* loop);
```
其实现是在DefaultPoller.cc文件之中，如下所示：
```
Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  if (::getenv("MUDUO_USE_POLL"))
  {
    return new PollPoller(loop);
  }
  else
  {
    return new EPollPoller(loop);
  }
}
```
可见除非定义了```MUDUO_USE_POLL```环境变量，否则muduo会使用epoll作为io复用的方式。

#### poll
因为epoll返回的io事件对应的结构体 epoll_event中，有一个data成员可以记录相关信息，可以方便我们找到对应的Channel，因而epoll的实现较poll比较更为简单。
```
typedef union epoll_data
{
  void* ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
}epoll_data_t;
```

```

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  LOG_TRACE << "fd total count " << channels_.size();
  int numEvents = ::epoll_wait(epollfd_,
                               &*events_.begin(),
                               static_cast<int>(events_.size()),
                               timeoutMs);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
    if (implicit_cast<size_t>(numEvents) == events_.size())
    {
      events_.resize(events_.size()*2);
    }
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << "nothing happended";
  }
  else
  {
    // error happens, log uncommon ones
    if (savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_SYSERR << "EPollPoller::poll()";
    }
  }
  return now;
}
```
拿到epoll返回的事件列表之后，就可以调用fillActiveChannels来将当前active的Channel填充进入EventLoop之中的activeChannels之中。
```
void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const
{
  assert(implicit_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i)
  {
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
    int fd = channel->fd();
    ChannelMap::const_iterator it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif
    // set revents
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}
```

#### update Channel
在Poller中使用```std::map<int, Channel*>``` 来存放fd和对应channel之间的对应关系。需要保证channel和fd的一一对应，不能存在一个fd由两个不同的channel来管理。

```
void EPollPoller::updateChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_TRACE << "fd = " << channel->fd()
    << " events = " << channel->events() << " index = " << index;
  if (index == kNew || index == kDeleted)
  {
    // a new one, add with EPOLL_CTL_ADD
    int fd = channel->fd();
    if (index == kNew)
    {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    }
    else // index == kDeleted
    {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  }
  else
  {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent())
    {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    }
    else
    {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}
```
Channel默认的index被设为-1，在epoll之中，index被用作标明类别，有以下3种类型，故而新创建的Channel其index代表新增的Channel。
```
namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}
```

updateChannel会根据Channel的revent来调用update方法。注意到对于没有关注任何io事件的Channel，EPollPoller采取的方式是将该channel对应的fd直接从epoll之中删除。

```
void EPollPoller::update(int operation, Channel* channel)
{
  struct epoll_event event;
  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
    << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
  {
    if (operation == EPOLL_CTL_DEL)
    {
      LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
    }
    else
    {
      LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
    }
  }
}
```

在update中，对于delete操作进行了区分处理，它容忍delete出错，但是对于modify和add出错，程序会fatal。

#### removeChannel
removeChannel将Channel对应的fd从epoll中删除，并且Channel也从channels_之中删除了。
```

void EPollPoller::removeChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded)
  {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

```

