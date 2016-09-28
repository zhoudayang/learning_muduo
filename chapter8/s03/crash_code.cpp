//
// Created by zhouyang on 16-9-28.
//
#include "EventLoop.h"

muduo::EventLoop *g_loop;

void print() {
    printf("hello world!\n");
}

void threadFunc() {
    g_loop->runAfter(1.0,print);
}

int main() {
    muduo::EventLoop loop;
    g_loop = &loop;
    muduo::Thread t(threadFunc);
    t.start();
    loop.loop();
}
