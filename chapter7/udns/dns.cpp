//
// Created by zhouyang on 16-9-21.
//

#include "Resolve.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

EventLoop * g_loop;
int count = 0;
int total = 0;

void quit(){
    g_loop->quit();
}

void resolveCallback(const string & host,const InetAddress &addr){
    LOG_INFO<<"resolved "<< host<< " -> "<<addr.toIp();
    if(++count == total)
        quit();
}

void resolve(Resolver * res,const string &host){
    res->resolve(host,boost::bind(&resolveCallback,host,_1));
}

int main(int argc, char ** argv){
    EventLoop loop;
    loop.runAfter(10,quit);
    g_loop = & loop;
    Resolver resolver (&loop);
    resolver.start();
    if(argc ==1){
        total ==3;
        resolve(&resolver ,"zhouyang.online");
        resolve(&resolver,"www.baidu.com");
        resolve(&resolver,"www.csdn.net");
    }
    else{
        total = argc-1;
        for(int i=1;i<argc;i++)
            resolve(&resolver,argv[i]);
    }
    loop.loop();
}