//
// Created by fit on 16-7-29.
//

#include "Channel.h"
#include "EventLoop.h"
#include <stdio.h>

#include <sstream>

#include <poll.h>

// add for osx support
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
#endif
// add for osx support

using namespace muduo;

const int Channel::kNoneEvent = 0;
// read event
const int Channel::kReadEvent = POLLIN | POLLPRI;
// write event
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fdArg)
        : loop_(loop), fd_(fdArg), events_(0), revents_(0), index_(-1) {}

//todo meaning what?
void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::handleEvent() {
    //POLLNVAL is equivalent to EBADF: it means the file descriptor does not actually refer to any open file,
    // i.e. it was closed or never open to begin with. This can never happen except as a result of a programming error or intentional attempt to query whether a file descriptor is invalid.
    if (revents_ & POLLNVAL) {
        printf("Channel::handle_event() POLLNVAL\n");
    }
    //there is an error
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_)
            errorCallback_();
    }
    // read event
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_)
            readCallback_();
    }
    //write event
    if (revents_ & POLLOUT) {
        if (writeCallback_)
            writeCallback_();
    }
}