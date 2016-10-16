//
// Created by zhouyang on 16-10-15.
//
#include <stdio.h>

#include "EventLoop.h"
#include "TimerId.h"

muduo ::EventLoop * g_loop;
muduo::TimerId toCancel;
int count  = 0;
void cancelSelf(){
    printf("called cancelSelf()\n");
    if(count++ == 5) {
        printf("cancel this Time callback function call\n");
        g_loop->cancel(toCancel);
    }
}

int main(){
    muduo::EventLoop loop;
    g_loop = & loop;
    toCancel = loop.runEvery(1,cancelSelf);
    loop.loop();
}