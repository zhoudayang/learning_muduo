//
// Created by fit on 16-10-16.
//

#include "Epoller.h"

#include "Channel.h"
#include "base/Logging.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

using namespace muduo;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
BOOST_STATIC_ASSERT(EPOLLIN==POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI==POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT==POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP==POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR==POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP==POLLHUP);

namespace {
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}

Epoller::Epoller(EventLoop* loop)
        :ownerLoop_(loop),
         //create epoll file descriptor
         epollfd_(::epoll_create(EPOLL_CLOEXEC)),
         events_(kInitEventListSize)
{
    //if error occurred, log and abort
    if (epollfd_<0)
    {
        LOG_SYSFATAL << "Epoller::Epoller";
    }

}

Epoller::~Epoller()
{
    //close epoll file descriptor
    ::close(epollfd_);
}

Timestamp Epoller::poll(int timeOutMs, ChannelList* activeChannels)
{
    /*
         int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);
         收集在epoll监控的事件中已经发送的事件。参数events是分配好的epoll_event结构体数组，epoll将会把发生的事件赋值到events数组中
         （events不可以是空指针，内核只负责把数据复制到这个events数组中，不会去帮助我们在用户态中分配内存）。
         maxevents告之内核这个events有多大，这个 maxevents的值不能大于创建epoll_create()时的size，
         参数timeout是超时时间（毫秒，0会立即返回，-1将不确定，也有说法说是永久阻塞）。
         如果函数调用成功，返回对应I/O上已准备好的文件描述符数目，如返回0表示已超时。
     */
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeOutMs);
    Timestamp now(Timestamp::now());
    if (numEvents>0)
    {
        LOG_TRACE << numEvents << " events happended";
    }
    fillActiveChannels(numEvents, activeChannels);
    //如果返回的监控的事件数目达到events_的容量，则对events_进行扩容处理
    if (implicit_cast<size_t>(numEvents)==events_.size())
    {
        events_.resize(events_.size()*2);
    }
    else if (numEvents==0)
    {
        LOG_TRACE << "nothing happended";
    }
    else
    {
        LOG_SYSERR << "Epoller::poll()";
    }
    //return poll return time
    return now;
}

//将活动的
void Epoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    assert(implicit_cast<size_t>(numEvents)<events_.size());
    for (int i = 0; i<numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it!=channels_.end());
        assert(it->second==channel);
#endif
        //set触发事件
        channel->set_revents(events_[i].events);
        //将这些active channel加入activeChannels
        activeChannels->push_back(channel);
    }
}

void Epoller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events " << channel->events();
    const int index = channel->index();
    if (index==kNew || index==kDeleted)
    {
        int fd = channel->fd();
        if (index==kNew)
        {
            assert(channels_.find(fd)==channels_.end());
            channels_[fd] = channel;
        }
        else
        {
            assert(channels_.find(fd)!=channels_.end());
            assert(channels_[fd]==channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        assert(channels_.find(fd)!=channels_.end());
        assert(channels_[fd]==channel);
        assert(index==kAdded);
        if (channel->isNoneEvent())
        {
            //don't care fd any more
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            //modify channel
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Epoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd)!=channels_.end());
    assert(channels_[fd]==channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index==kAdded || index==kDeleted);
    //erase it from channels_
    size_t n = channels_.erase(fd);
    (void) n;
    assert(n==1);
    if (index==kAdded)
    {
        //delete it from epoll
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void Epoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    //set channel * ptr to event.data.ptr
    event.data.ptr = channel;
    int fd = channel->fd();
    //update struct epoll_event in epoll
    if (::epoll_ctl(epollfd_, operation, fd, &event)<0)
    {
        if (operation==EPOLL_CTL_DEL)
        {
            LOG_SYSERR << "epoll_ctl op = " << operation << " fd = " << fd;
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl op = " << operation << " fd = " << fd;
        }
    }
}