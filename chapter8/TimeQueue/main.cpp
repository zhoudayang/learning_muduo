#include "EventLoop.h"　
#include <boost/bind.hpp>

#include <stdio.h>

int cnt =0;

muduo::EventLoop *g_loop;

void printTid(){
    printf("pid = %d,tid = %u\n",getpid(),muduo::CurrentThread::tid());
    printf("now %s\n",muduo::Timestamp::now().toString().c_str());
}

void print(const char *msg){
    printf("msg %s %s \n",muduo::Timestamp::now().toString().c_str(),msg);
    if(++cnt==20){//如果该函数调用次数达到20次，调用quit方法，退出loop 循环
        g_loop->quit();
    }
}

int main(){
    printTid();
    muduo::EventLoop loop;
    g_loop=&loop;

    print("main");

//    loop.runAfter(1,boost::bind(print,"once1"));
//    loop.runAfter(1.5,boost::bind(print,"once1.5"));
//    loop.runAfter(2.5,boost::bind(print,"once2.5"));
//    loop.runAfter(3.5,boost::bind(print,"once3.5"));
    loop.runEvery(2,boost::bind(print,"every 2"));
//    loop.runEvery(3,boost::bind(print,"every 3"));

    loop.loop();
    print("main loop exists");
    sleep(1);
}