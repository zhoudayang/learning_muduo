#include <iostream>
#include <thread>
#include "Thread.h"

using namespace std;

void threadFunc(){

    printf("threadFunc: pid = %d, tid = %d\n",getpid(),muduo::CurrentThread::tid());
}
int main(){
    thread t1(threadFunc);
    thread t2(threadFunc);
    thread t3(threadFunc);
    t1.join();
    t2.join();
    t3.join();

    return 0;
}
