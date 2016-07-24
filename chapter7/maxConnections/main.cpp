#include "echo.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

int main() {
    LOG_INFO << " pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(2016);
    //set max connection to 200
    EchoServer server(&loop, listenAddr,200);
    server.start();
    loop.loop();
    return 0;
}
