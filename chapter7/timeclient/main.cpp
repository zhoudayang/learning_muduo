#include "timeclient.h"




int main() {
    LOG_INFO << "pid = " << getpid();
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 2016);
    TimeClient timeClient(&loop, serverAddr);
    timeClient.connect();
    loop.loop();
}