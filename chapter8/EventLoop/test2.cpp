//
// Created by fit on 16-7-30.
//

#include "EventLoop.h"
#include "Thread.h"

muduo::EventLoop * g_loop;

void threadFunc(){
    g_loop->loop();
}

int main(){
    muduo::EventLoop loop;
    g_loop = &loop;
    muduo::Thread t(threadFunc,"thread");
    t.start();
    t.join();

    return 0;
}