//
// Created by zhouyang on 16/8/1.
//

#include "Channel.h"
#include "EventLoop.h"

#include <sstream>

#include <poll.h>

using namespace muduo;

// add for osx support
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif
#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
#endif
// add for osx support

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1) { }

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::handleEvent() {
    if (revents_ & POLLNVAL) {
        printf("Channel::handle_events() POLLNVAL\n");
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_)
            readCallback_();
    }
    if (revents_ & POLLOUT) {
        if (writeCallback_)
            writeCallback_();
    }
}