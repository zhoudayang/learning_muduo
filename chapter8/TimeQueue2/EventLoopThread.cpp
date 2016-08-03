//
// Created by zhouyang on 16-8-3.
//

#include "EventLoopThread.h"
#include "EventLoop.h"
#include <boost/bind.hpp>

using namespace muduo;
using std::unique_lock;
EventLoopThread::EventLoopThread()
        : loop_(NULL),
          exiting_(false),
          thread_(boost::bind(&EventLoopThread::threadFunc, this))
{
}
EventLoopThread::~EventLoopThread() {
    exiting_= true;
    loop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();
    {
        unique_lock<mutex> lock(mutex_);
        while(loop_==NULL){
            cond_.wait(lock);
        }
    }
    return loop_;
}
void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        unique_lock<mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
}