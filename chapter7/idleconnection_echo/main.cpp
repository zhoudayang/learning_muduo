#include "echo.h"
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <boost/functional/hash/hash.hpp>
#include <muduo/net/InetAddress.h>


using namespace muduo;
using namespace muduo::net;

//两个shared_ptr 如果指向不同的对象，那么他们的hash值不相同
//如果指向相同的对象，他们的值相同
//如果都不指向任何对象，他们的值也相同
void testHash() {
    boost::hash<boost::shared_ptr<int> > h;
    boost::shared_ptr<int> x1(new int(10));
    boost::shared_ptr<int> x2(new int(10));
    h(x1);
    assert(h(x1) != h(x2));
    x1 = x2;
    assert(h(x1) == h(x2));
    x1.reset();
    assert(h(x1) != h(x2));
    x2.reset();
    assert(h(x1) == h(x2));
}

int main() {
    testHash();
    EventLoop loop;
    InetAddress listenAddress(2008);
    int idleSeconds = 10;
    LOG_INFO << "pid = " << getpid() << ", idle seconds = " << idleSeconds;
    EchoServer server(&loop, listenAddress, idleSeconds);
    server.start();
    loop.loop();
}