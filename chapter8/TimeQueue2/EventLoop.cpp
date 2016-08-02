//
// Created by zhouyang on 16/8/1.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"

#include <stdio.h>

#include <assert.h>

using namespace muduo;

__thread EventLoop *t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

EventLoop::EventLoop() : looping_(false), quit_(false), threadId_(CurrentThread::tid()), poller_(new Poller(this)),
                         timerQueue_(new TimerQueue(this)) {
    printf("EventLoop created %p in thread %u \n", this, threadId_);
    //是否有其他的EventLoop 绑定到这个线程
    if (t_loopInThisThread) {
        printf("Another EventLoop thread %p existd in this thread %u", t_loopInThisThread, threadId_);

    }
    else
        t_loopInThisThread = this;
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
        //清空activeChannels_
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        //调用回调函数
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); it++) {
            (*it)->handleEvent();
        }
    }
    printf("EventLoop %p stop looping\n", this);
    looping_ = false;
}


void EventLoop::quit() {
    quit_ = true;
}

TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb) {
    //增加定时器
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    //在time这一时刻运行
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    //interval is not null,repeat is true
    return timerQueue_->addTimer(cb, time, interval);
}

//更新channel关注的事件　或者插入新的channel
void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);

}

void EventLoop::abortNotInLoopThread() {
    printf("EventLoop::abortNotInLoopThread - EventLoop %p was created in threadId_= %u ,current thread id = %u\n",
           this, threadId_, CurrentThread::tid());
    abort();
}