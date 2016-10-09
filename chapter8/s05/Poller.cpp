//
// Created by zhouyang on 16-9-27.
//

#include "Poller.h"

#include "Channel.h"
#include "base/Logging.h"

#include <assert.h>
#include <poll.h>

using namespace muduo;

Poller::Poller(EventLoop *loop) :
        ownerLoop_(loop) {

}


Poller::~Poller() {

}


Timestamp Poller::poll(int timeoutMs, ChannelList *activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE <<numEvents<< " events happened";
        //fill events that happened of channel into activeChannels list
        fillActiveChannels(numEvents,activeChannels);
    }
    else if(numEvents ==0){
        LOG_TRACE<<" nothing happened";
    }
    else{
        LOG_SYSERR<<"Poller::poll()";
    }
    return now;
}


void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for(PollFdList::const_iterator pfd = pollfds_.begin();pfd!=pollfds_.end() && numEvents >0;++pfd){
        if(pfd->revents>0){
            --numEvents;
            //key fd, value Channel *
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch!=channels_.end());
            Channel * channel = ch->second;
            assert(channel->fd() == pfd->fd);
            //set flag to show that events happened
            channel->set_revents(pfd->revents);
            //store active channels
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel *channel) {
    assertInLoopThread();
    LOG_TRACE<<"fd = "<<channel->fd()<< " events = "<<channel->events();
    //append new into pollfds_
    if(channel->index() < 0){
        assert(channels_.find(channel->fd())==channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size())-1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
        //refresh pollfd in pollfds_ according to channel
    else
    {
        assert(channels_.find(channel->fd())!=channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0<=idx && idx<static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd ==channel->fd() || pfd.fd ==-1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        //如果channel关注的事件为空，那么设置pollfd 中的fd为-1,　让poll暂时忽略此项
        if(channel->isNoneEvent()){
            pfd.fd=-1;
        }
    }
}