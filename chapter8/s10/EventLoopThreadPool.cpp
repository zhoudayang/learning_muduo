//
// Created by fit on 16-10-13.
//

#include "EventLoopThreadPool.h"

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <boost/bind.hpp>

using namespace muduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop) :
        baseLoop_(baseLoop),
        started_(false),
        numThreads_(0),
        next_(0) { }

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::start()
{
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;
    //init threadpool with numThreads_ size
    for (int i = 0; i<numThreads_; i++)
    {
        EventLoopThread* t = new EventLoopThread();
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}

//如果n为0,返回 baseLoop_, 如果n为1,返回loops_中仅有的那个loop
//相对应的，如果n大于1,那么一次轮流返回loops_容器中的loop
EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;
    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t> (next_)>=loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}