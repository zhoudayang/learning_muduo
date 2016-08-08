//
// Created by zhouyang on 16/8/1.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include <boost/bind.hpp>
#include <stdio.h>
#include <sys/eventfd.h>
#include <assert.h>

using namespace muduo;

__thread EventLoop *t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

//创建event fd 用于在loop中运行给定函数
static int createEventFd() {
    //noblocking  EFD_NONBLOCK非阻塞
    //调用exec之后会自动关闭文件描述符，防止泄露
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    //创建文件描述符失败
    if (evtfd < 0) {
        printf("failed in eventfd\n");
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop() :
        looping_(false),
        quit_(false),
        //waiting for functor 仿函数
        callingPendingFunctors_(false),
        threadId_(CurrentThread::tid()),
        poller_(new Poller(this)),
        timerQueue_(new TimerQueue(this)),
        wakeupFd_(createEventFd()),
        wakeupChannel_(new Channel(this, wakeupFd_)) {

    printf("EventLoop created %p in thread %u \n", this, threadId_);
    //是否有其他的EventLoop 绑定到这个线程
    if (t_loopInThisThread) {
        printf("Another EventLoop thread %p existd in this thread %u", t_loopInThisThread, threadId_);

    }
    else
        t_loopInThisThread = this;
    //wakeup channel set read callback function
    wakeupChannel_->setReadCallback(boost::bind(&EventLoop::handleRead, this));
    //we are always reading the wakeupfd_
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    //not begin loop now
    assert(!looping_);
    //close event file description
    ::close(wakeupFd_);
    //避免空悬指针
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
        doPendingFunctors();
    }
    printf("EventLoop %p stop looping\n", this);
    looping_ = false;
}


void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread())
        wakeup();
}

void EventLoop::runInLoop(const Functor &cb) {
    //in loop thread ,call callback function immediately
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}

//push cb to pendingFunctors,run after loop done
void EventLoop::queueInLoop(const Functor &cb) {
    {
        //保护pendingFunctors_
        std::unique_lock<std::mutex> lock(mutex_);
        //push cb into back of pendingFunctors_
        PendingFunctors_.push_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_)
        //set eventfd to 1, wake up loop thread to run the callback function
        wakeup();
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

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    printf("EventLoop::abortNotInLoopThread - EventLoop %p was created in threadId_= %u ,current thread id = %u\n",
           this, threadId_, CurrentThread::tid());
    abort();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    //reset event fd to 1
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        printf("EventLoop::wakeup() reads %d bytes instead of 8 \n", n);
    }
}

//eventfd callback function
void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        printf("EventLoop::handleRead() reads %d bytes instead of 8 \n", n);
    }
}

//calling functions store in PendingFunctors_
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    //now calling pending functors, set flag to true
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        //swap functors and pendingFunctos_ , now pendingFunctors_ is an empty vector
        functors.swap(PendingFunctors_);
    }
    for (size_t i = 0; i < functors.size(); i++) {
        //calling functors
        functors[i]();
    }
    //now calling pending functors, set flag to false
    callingPendingFunctors_ = false;
}