//
// Created by zhouyang on 16/8/1.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"

#include <assert.h>

using namespace muduo;

__thread EventLoop *t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

EventLoop::EventLoop() : looping_(false), quit_(false), threadId_(CurrentThread::tid()), poller_(new Poller(this)),
                         timerQueue_(new TimerQueue(this)) {
    printf("EventLoop created\n");
    if (t_loopInThisThread) {
        printf("Another EventLoop thread %p existd in this thread %u", t_loopInThisThread, threadId_);

    }
    else
        t_loopInThisThread = false;
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); it++) {
            (*it)->handleEvent();
        }
        printf("EventLoop %p stop looping\n", this);
        looping_ = false;
    }
}


void EventLoop::quit() {
    quit_ = true;
}

TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb) {
    return timerQueue_->addTimer(cb,time,0.0);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(),interval));
    return timerQueue_->addTimer(cb,time,interval);
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop()==this);
    assertInLoopThread();
    poller_->updateChannel(channel);

}