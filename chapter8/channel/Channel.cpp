//
// Created by fit on 16-7-29.
//

#include "Channel.h"
#include "EventLoop.h"
#include <stdio.h>

#include <sstream>

#include <poll.h>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fdArg)
        : loop_(loop), fd_(fdArg), events_(0), revents_(0), index_(-1) {}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::handleEvent() {
    if (revents_ & POLLNVAL) {
        printf("Channel::handle_event() POLLNVAL\n");
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