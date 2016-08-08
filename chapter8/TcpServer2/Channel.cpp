//
// Created by zhouyang on 16/8/1.
//

#include "Channel.h"
#include "EventLoop.h"
#include "logging/logging.h"

#include <poll.h>

using namespace muduo;
//cares about nothing
const int Channel::kNoneEvent = 0;
//cares about read event
const int Channel::kReadEvent = POLLIN | POLLPRI;
//cares about write event
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
        : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), eventHandling_(false) {}

Channel::~Channel() {
    //断言在事件处理期间，本channel 对象不会析构
    //否则会引起core dump
    assert(!eventHandling_);
}

void Channel::update() {
    //refresh channel
    loop_->updateChannel(this);
}


void Channel::handleEvent() {
    //now I am handling the event
    eventHandling_ = true;
    //an error happened
    if (revents_ & POLLNVAL) {
        printf("Channel::handle_events() POLLNVAL\n");
    }
    //close the file descripter
    if((revents_&POLLHUP) &&(revents_&POLLIN)){
        LOG_WARN<<"Channel::handle_event POLLHUP";
        if(closeCallback_){
            closeCallback_();
        }
    }
    //call error handler
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_)
            errorCallback_();
    }
    //call read callback
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_)
            readCallback_();
    }
    //call write callback
    if (revents_ & POLLOUT) {
        if (writeCallback_)
            writeCallback_();
    }
    //now I am not handling the event
    eventHandling_ = false;
}