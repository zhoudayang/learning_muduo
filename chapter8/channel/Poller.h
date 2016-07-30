//
// Created by fit on 16-7-29.
//

#ifndef CHANNEL_POLLER_H
#define CHANNEL_POLLER_H

#include <map>
#include <vector>


#include "EventLoop.h"
#include "datetime/Timestamp.h"


/*
  Data structure describing a polling request.
    struct pollfd
    {
        int fd;			        // File descriptor to poll.
        short int events;		//Types of events poller cares about.
        short int revents;		//Types of events that actually occurred.
    };

*/

struct pollfd;



// IO Multiplexing with poll(2)
//this class doesn't own the Channel objects
namespace muduo {
    class Channel;

    class Poller : boost::noncopyable {
    public:
        typedef std::vector<Channel *> ChannelList;

        Poller(EventLoop *loop);

        ~Poller();

        //polls the I/O events
        //must be called in the loop thread
        Timestamp poll(int timeoutMs, ChannelList *activeChannels);


        //Changes the interested I/O events
        //Must be called in the loop thread
        void updateChannel(Channel *channel);


        // assert if it is in loop thread?
        void assertInLoopThread() {
            ownerLoop_->assertInLoopThread();
        }

    private:
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel *> ChannelMap;

        EventLoop *ownerLoop_;
        //pollfd list
        PollFdList pollfds_;
        //map key:fd value:Channel *
        ChannelMap channels_;

    };
}


#endif //CHANNEL_POLLER_H
