//
// Created by zhouyang on 16-9-28.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

#include <boost/bind.hpp>

using namespace muduo;


EventLoopThread::EventLoopThread():
    loop_(NULL),
    existing_(false),
    thread_(boost::bind(&EventLoopThread::threadFunc,this)),
    mutex_(),
    cond_(mutex_)
{

}
EventLoopThread::~EventLoopThread() {
    existing_ = true;
    //quit loop
    loop_->quit();
    //join the thread, wait to exit event loop
    thread_.join();
}

//call this function in other thread
EventLoop * EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        //wait until loop_ is not NULL
        while(loop_==NULL){
            cond_.wait();
        }
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        MutexLockGuard lock(mutex_);
        loop_= &loop;
        //notify startLoop that loop init complete!
        cond_.notify();
    }
    //start the event loop
    loop.loop();
}