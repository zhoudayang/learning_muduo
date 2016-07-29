//
// Created by fit on 16-7-28.
//

#include "EventLoop.h"

#include <assert.h>
#include <poll.h>

using namespace muduo;

__thread EventLoop *t_loopInThisThread = 0;

EventLoop::EventLoop() : looping_(false), threadId_(CurrentThread::tid()) {
    printf("EventLoop created %p in thread \n", threadId_);
    if (t_loopInThisThread) {
        printf("Another EventLoop %p exists in this thread %ld \n", t_loopInThisThread, threadId_);
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    ::poll(NULL, 0, 5 * 1000);
    printf("EventLoop %p stop looping\n", t_loopInThisThread);
    looping_ = false;
}

void EventLoop::abortNotInloopThread() {
    printf("EventLoop::abortNotInloopThread -EventLoop %p was created in threadId_ = %ld ,current thread id = %ld",
           this, threadId_, CurrentThread::tid());
}