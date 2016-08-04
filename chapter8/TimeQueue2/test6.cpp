//
// Created by fit on 16-8-4.
//

#include "EventLoop.h"
#include "EventLoopThread.h"
#include <stdio.h>

void runInThread(){
    printf("runInThread():pid = %d,tid = %d\n",getpid(),muduo::CurrentThread::tid());
}

int main(){
    printf("main(): pid = %d,tid = %d\n",getpid(),muduo::CurrentThread::tid());
    muduo::EventLoopThread loopThread;
    muduo::EventLoop *loop = loopThread.startLoop();
    loop->runInLoop(runInThread);
    sleep(1);
    loop->runInLoop(runInThread);
    sleep(3);
    loop->quit();
    printf("exit main(). \n");


}
