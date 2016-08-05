//
// Created by zhouyang on 16/8/1.
//

#ifndef POLLER_H
#define POLLER_H

#include <map>
#include <vector>
#include <unistd.h>
#include "datetime/Timestamp.h"
#include "EventLoop.h"
#include <boost/noncopyable.hpp>

struct pollfd;
namespace muduo {
    class Channel;

    //this class doesn;t own the Channel objects
    class Poller : boost::noncopyable {
    public:
        typedef std::vector<Channel *> ChannelList;

        Poller(EventLoop *loop);

        ~Poller();

        //polls the IO events
        //must be called in the loop thread
        Timestamp poll(int timeoutMs, ChannelList *activeChannels);

        //changes the interestd io events
        void updateChannel(Channel *channel);

        void assertInLoopThread() {
            ownerLoop_->assertInLoopThread();
        }


    private:
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;


        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel *> ChannelMap;

        EventLoop *ownerLoop_;
        PollFdList pollfds_;;
        ChannelMap channels_;
    };
}


#endif //TIMEQUEUE_POLLER_H
