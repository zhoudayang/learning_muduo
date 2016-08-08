//
// Created by zhouyang on 16/8/1.
//

#include "Poller.h"

#include "Channel.h"
#include <assert.h>
#include <poll.h>
#include <stdio.h>

using namespace muduo;

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}

Poller::~Poller() {}

Timestamp Poller::poll(int timeoutMs, ChannelList *activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        printf("%d events happended\n", numEvents);
        //fill active events into activeChannels
        fillActiveChannels(numEvents, activeChannels);
    }
    else if (numEvents == 0) {
        printf("nothing happended\n");
    }
    else {
        printf("Poller::poll()\n");
    }
    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; pfd++) {
        if (pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            //events happended , set revents_
            channel->set_revents(pfd->revents);
            //push channel to activeChannels
            activeChannels->push_back(channel);

        }
    }
}

//重置channels_ 设置新的events
void Poller::updateChannel(Channel *channel) {
    assertInLoopThread();
    printf("fd = %d ,events = %d\n", channel->fd(), channel->events());
    if (channel->index() < 0) {
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        //set events that cares about
        pfd.events = static_cast<short> (channel->events());
        //set revents to zero
        pfd.revents = 0;
        //push pfd to the end of the pollfds_
        pollfds_.push_back(pfd);
        //set index
        int idx = pollfds_.size() - 1;
        channel->set_index(idx);
        //key fd value channel
        channels_[pfd.fd] = channel;
    }
    else {
        //assert channels_ exists channel
        assert(channels_.find(channel->fd()) != channels_.end());
        //assert channels_[channel->fd()]==channel
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        //assert idx lines between 0 and pollfds_.size()-1
        assert(idx >= 0 && idx < static_cast<int> (pollfds_.size()));
        struct pollfd &pfd = pollfds_[channel->fd()];
        //assert fd is right
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        //set events
        pfd.events = static_cast<short>(channel->events());
        //reset revents to zero
        pfd.revents = 0;
        //如果某一个Channel暂时不关心任何是时间，就将pollfd.fd设为-1,让poll(2)忽略此项
        if (channel->isNoneEvent())
            pfd.fd = -1;
    }
}

void Poller::removeChannel(Channel *channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd &pfd = pollfds_[idx];
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1);
    if (implicit_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    } else {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}