# Poller
#### Poller详解
在poller之中，定义了一些接口, 如下所示：

```
/// Polls the I/O events.
/// Must be called in the loop thread.
virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

/// Changes the interested I/O events.
/// Must be called in the loop thread.
virtual void updateChannel(Channel* channel) = 0;

/// Remove the channel, when it destructs.
/// Must be called in the loop thread.
virtual void removeChannel(Channel* channel) = 0;
```
其中，poll用于对应io复用之中的poll调用，并且将当前有io事件发生的Channel插入到activeChannels之中。updateChannel用于更新Channel， removeChannel则是从Poller之中移除Channel。

为了记录fd和Channel之间的对应关系，这里使用了std::map，如下所示：
```
typedef std::map<int, Channel*> ChannelMap;
ChannelMap channels_;
```

定义此接口类为了抽象出Poller的主要运行逻辑，后续根据poll和epoll模型实现出对应的Poller。使得整个网络库能够同时支持poll和epoll网络模型。


