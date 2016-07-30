//
// Created by zhouyang on 16-7-29.
//

#include "Poller.h"

#include "Channel.h"

#include <assert.h>
#include <poll.h>

using namespace muduo;

//construction
Poller::Poller(EventLoop *loop)
        : ownerLoop_(loop) {}

Poller::~Poller() {}

//return ChannelList which contains cared occured events
Timestamp Poller::poll(int timeoutMs, ChannelList *activeChannels) {
    //todo    poll function
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        printf("%d events happended\n", numEvents);
        //now have numEvents active Channels
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        printf("nothing happended!\n");
    } else {
        printf("Poller::poll()\n");
    }
    return now;
}
//遍历pollfds_，找到有活动的事件的fd,把它对应的Channel填入activeChannels
//refresh active channel list
void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        //Types of events that actually occurred.
        if (pfd->revents > 0) {
            --numEvents;
            //fd: File descriptor to poll
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            // active channel list add one
            activeChannels->push_back(channel);
        }
    }
}

//处理channel订阅的事件和初始化等问题
void Poller::updateChannel(Channel *channel) {
    assertInLoopThread();
    printf("fd = %d events = %d\n", channel->fd(), channel->events());
    //a new one, add to pollfds_
    if (channel->index() < 0) {
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        //set index
        channel->set_index(idx);
        //key fd value channel
        channels_[pfd.fd] = channel;
    } else {
        //update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short> (channel->events());
        //reset revents to 0
        pfd.revents = 0;
        if (channel->isNoneEvent())
            //如果某一个Channel暂时不关心任何是时间，就将pollfd.fd设为-1,让poll(2)忽略此项
            pfd.fd = -1;
    }
}
