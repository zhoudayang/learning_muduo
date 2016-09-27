//
// Created by zhouyang on 16-9-27.
//

#include "EventLoop.h"
#include "base/Thread.h"

muduo::EventLoop * g_loop;

void threadFunc(){
    g_loop->loop();
}

//运行loop的线程和EventLoop 创建的线程不一致，程序断言失败，直接退出。
int main(){
    muduo::EventLoop loop;
    g_loop = &loop;
    muduo::Thread t(threadFunc);
    t.start();
    t.join();
    return 0;
}