#include "chargen.h"

#include <muduo/base/Logging.h>
#include<muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

//chargen 协议
//只发送数据，不接收数据，而且发送数据的速度不能快过客户端接收的速度

int main() {

    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    InetAddress listenAddr(2016);
    ChargenServer server(&loop, listenAddr, true);
    server.start();
    loop.loop();
}