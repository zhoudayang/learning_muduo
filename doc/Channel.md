#### 实现简介


channel一般用于抽象一个fd，包含fd当前关注的事件，出现事件更新需要在poller之中做出更新。poller获取到当前io复用返回的io事件之后，会更新channel之中的revent。在eventloop中会为当前活跃的channel进行事件处理，调用对应revent，io事件的回调函数。整个Channel类的实现非常简单明了。


#### 重点:tie

可能出现这种情况，我们设置的回调函数是某个类的成员函数，此类对象使用shared_ptr进行管理。如果在回调函数之中对此shared_ptr进行了reset或者release处理，那么此回调函数就会立即失效，因为类对象被回收了。对于这种特殊情况，可以使用```std::weak_ptr<void> tie```来存放该shared_ptr的弱引用（使用弱引用是为了避免增加shared_ptr的引用计数，导致对象一直无法得以析构。每次进行事件处理的时候，若之前设置了tie，那么首先tie进行提升，保证回调函数处理期间此shared_ptr管理的对象不会被析构。


tie这一措施是为了TcpConnectionPtr而设计了，如果在某个回调函数之中，reset了此TcpConnectionPtr, 那么TcpConnection的其他回调函数就会失效。所以在处理这些事件期间，需要提升此TcpConnectionPtr的引用计数，阻止TcpConnectionPtr被析构。


```
void Channel::tie(const std::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Channel::handleEvent(Timestamp receiveTime)
{
  std::shared_ptr<void> guard;
  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      handleEventWithGuard(receiveTime);
    }
  }
  else
  {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
  eventHandling_ = true;
  LOG_TRACE << reventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
  {
    if (logHup_)
    {
      LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (closeCallback_) closeCallback_();
  }

  if (revents_ & POLLNVAL)
  {
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL))
  {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
  {
    if (readCallback_) readCallback_(receiveTime);
  }
  if (revents_ & POLLOUT)
  {
    if (writeCallback_) writeCallback_();
  }
  eventHandling_ = false;
}

```
