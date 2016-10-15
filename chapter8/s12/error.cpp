
#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"


//当服务器close一个连接时，若client端接着发数据。根据TCP协议的规定，会收到一个RST响应，client再往这个服务器发送数据时，系统会发出一个SIGPIPE信号给进程，告诉进程这个连接已经断开了，不要再写了。
//根据信号的默认处理规则SIGPIPE信号的默认执行动作是terminate(终止、退出),所以client会退出。若不想客户端退出可以把SIGPIPE设为SIG_IGN
//

muduo::string message1;
muduo::string message2;

int sleepSeconds = 0;
//错误：假设sleepSeconds是５秒，用nc localhost 9981 创建连接之后立即退出nc,服务进程过了几秒就会退出。
void onConnection(const muduo::TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s", conn->name().c_str(),
                conn->peerAddress().toHostPort().c_str());
        if (sleepSeconds>0)
        {
            ::sleep(sleepSeconds);
        }
        conn->send(message1);
        conn->send(message2);
        conn->shutdown();
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
    }
}

void onMessage(const muduo::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n ", buf->readableBytes(), conn->name().c_str(),
            receiveTime.toFormattedString().c_str());
    buf->retrieveAll();
}

int main(int argc, char** argv)
{
    printf("main() :pid = %d\n", getpid());

    int len1 = 100;
    int len2 = 200;

    if (argc>2)
    {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }
    if (argc>3)
    {
        sleepSeconds = atoi(argv[3]);
    }

    message1.resize(len1);
    message2.resize(len2);

    std::fill(message1.begin(), message1.end(), 'A');
    std::fill(message2.begin(), message2.end(), 'B');

    muduo::InetAddress listenAddr(9981);

    muduo::EventLoop loop;

    muduo::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    server.start();
    loop.loop();

    return 0;
}