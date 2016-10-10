//
// Created by zhouyang on 16-9-27.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"

#include "base/Logging.h"

#include <assert.h>

#include <sys/eventfd.h>
#include <boost/bind.hpp>

using namespace muduo;

__thread EventLoop *t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

//create event file descriptor
static int createEventfd() {
    int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventFd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return eventFd;
}


EventLoop::EventLoop() :
        looping_(false),
        quit_(false),
        threadId_(CurrentThread::tid()),
        poller_(new Poller(this)),
        timerQueue_(new TimerQueue(this)),
        wakeupFd_(createEventfd()),
        wakeupChannel_(new Channel(this, wakeupFd_)) {
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;
    }
    //set read call back function
    wakeupChannel_->setReadCallback(boost::bind(&EventLoop::handleRead, this));
    //enable to read
    wakeupChannel_->enableReading();
}


EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    //looping_　用于标明main　loop 是否正在运行
    looping_ = true;
    quit_ = false;
    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (ChannelList::const_iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            //let Channel handle the event that occurred
            (*it)->handleEvent();
        }
        //calling pending Functors
        doPendingFunctors();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
}

TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}


void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
    << " was created in threadId_ = " << threadId_
    << ", current thread id = " << CurrentThread::tid();
}

//可能在doPendingFunctors中调用
void EventLoop::runInLoop(const Functor &cb) {
    if (isInLoopThread())
    {
        //if is in io thread, call the callback function
        cb();
    }
    else
    {
        //add callback function into queue, call it in event loop
        queueInLoop(cb);
    }
}

//可能在doPendingFunctors中调用
void EventLoop::queueInLoop(const Functor &cb) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        //如果当前正在运行pending functors, 那么设置eventfd,　下次进入poll的时候就会立即返回
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    //write 1 to event file descriptor
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    //标明现在运行 pending functors
    callingPendingFunctors_ = true;
    //swap减小临界区
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (size_t i = 0; i < functors.size(); i++)
    {
        functors[i]();
    }
    //标明运行结束
    callingPendingFunctors_ = false;
}

void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}











