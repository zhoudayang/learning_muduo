#include "EventLoop.h"
#include "base/Thread.h"
#include <stdio.h>

void threadFunc(){
    printf("threadFunc(): pid = %d, tid = %d \n",getpid(),muduo::CurrentThread::tid());
    muduo::EventLoop loop;
    loop.loop();
}

int main(){
    //在主线程和子线程中创建一个EventLoop，程序正常运行之后退出
    printf("main(): pid = %d, tid = %d \n",getpid(),muduo::CurrentThread::tid());
    muduo::EventLoop loop;
    muduo::Thread thread(threadFunc);
    thread.start();
    loop.loop();
    pthread_exit(NULL);

}