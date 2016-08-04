//
// Created by fit on 16-8-4.
//

#include "EventLoop.h"
#include "Thread.h"
muduo::EventLoop* g_loop;

void print(){
    printf("print from print function\n");
}

//因为在非io线程中调用了runEvery函数，会导致程序crash
void threadFunc(){
    g_loop->runEvery(1.0,print);
}

int main(){
    muduo::EventLoop loop;
    g_loop = &loop;
    muduo::Thread t(threadFunc);
    t.start();
    loop.loop();
}