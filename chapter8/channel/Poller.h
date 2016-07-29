//
// Created by fit on 16-7-29.
//

#ifndef CHANNEL_POLLER_H
#define CHANNEL_POLLER_H

#include <map>
#include <vector>


#include "EventLoop.h"
#include "datetime/Timestamp.h"

struct pollfd;

namespace muduo {
    class Channel;

    class Poller : boost::noncopyable {
    public:
        typedef std::vector < Channel * > ChannelList;

        Poller(EventLoop *loop);

        ~Poller();

        Timestamp poll(int timeoutMs, ChannelList *activeChannels);

        void updateChannel(Channel *channel);

        void assertInLoopThread() {
            ownerLoop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel *> ChannelMap;

        EventLoop *ownerLoop_;
        PollFdList pollfds_;
        ChannelMap channels_;

    };
}


#endif //CHANNEL_POLLER_H
