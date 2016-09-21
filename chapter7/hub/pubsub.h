//
// Created by zhouyang on 16-9-21.
//
#ifndef _PUBSUB_
#define _PUBSUB_

#include "codec.h"

#include <muduo/net/TcpClient.h>
#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;




namespace pubsub
{
    using muduo::string;
    using muduo::Timestamp;

// FIXME: dtor is not thread safe
    class PubSubClient : boost::noncopyable
    {
    public:
        //connection callback function
        typedef boost::function<void (PubSubClient*)> ConnectionCallback;
        //Subscribe callback function
        typedef boost::function<void (const string& topic,
                                      const string& content,
                                      Timestamp)> SubscribeCallback;

        PubSubClient(muduo::net::EventLoop* loop,
                     const muduo::net::InetAddress& hubAddr,
                     const string& name);
        void start();

        void stop();

        bool connected() const;

        void setConnectionCallback(const ConnectionCallback& cb)
        { connectionCallback_ = cb; }

        bool subscribe(const string& topic, const SubscribeCallback& cb);

        void unsubscribe(const string& topic);

        bool publish(const string& topic, const string& content);

    private:
        void onConnection(const muduo::net::TcpConnectionPtr& conn);

        void onMessage(const muduo::net::TcpConnectionPtr& conn,
                       muduo::net::Buffer* buf,
                       muduo::Timestamp receiveTime);

        bool send(const string& message);

        muduo::net::TcpClient client_;
        muduo::net::TcpConnectionPtr conn_;
        ConnectionCallback connectionCallback_;
        SubscribeCallback subscribeCallback_;
    };
}
using namespace pubsub;

PubSubClient::PubSubClient(EventLoop* loop,
                           const InetAddress& hubAddr,
                           const string& name)
        : client_(loop, hubAddr, name)
{
    // FIXME: dtor is not thread safe
    client_.setConnectionCallback(
            boost::bind(&PubSubClient::onConnection, this, _1));
    client_.setMessageCallback(
            boost::bind(&PubSubClient::onMessage, this, _1, _2, _3));
}

//connect server
void PubSubClient::start()
{
    client_.connect();
}

//disconnect from server
void PubSubClient::stop()
{
    client_.disconnect();
}

//is now connected?
bool PubSubClient::connected() const
{
    return conn_ && conn_->connected();
}

bool PubSubClient::subscribe(const string& topic, const SubscribeCallback& cb)
{
    string message = "sub " + topic + "\r\n";
    subscribeCallback_ = cb;
    return send(message);
}

void PubSubClient::unsubscribe(const string& topic)
{
    string message = "unsub " + topic + "\r\n";
    send(message);
}

//publish callback function
bool PubSubClient::publish(const string& topic, const string& content)
{
    string message = "pub " + topic + "\r\n" + content + "\r\n";
    return send(message);
}

void PubSubClient::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        //store TcpConnectionPtr
        conn_ = conn;
        // FIXME: re-sub
    }
    else
    {
        conn_.reset();
    }
    //if set connection callback function, call it
    if (connectionCallback_)
    {
        connectionCallback_(this);
    }
}

void PubSubClient::onMessage(const TcpConnectionPtr& conn,
                             Buffer* buf,
                             Timestamp receiveTime)
{
    ParseResult result = kSuccess;
    while (result == kSuccess)
    {
        string cmd;
        string topic;
        string content;
        result = parseMessage(buf, &cmd, &topic, &content);
        if (result == kSuccess)
        {
            //服务器发布 pub指令
            if (cmd == "pub" && subscribeCallback_)
            {
                //调用subscribe 的回调函数
                subscribeCallback_(topic, content, receiveTime);
            }
        }
        else if (result == kError)
        {
            //shutdown the connection when error occurred
            conn->shutdown();
        }
    }
}

//向服务器发送message,　成功发送返回true, 失败返回false
bool PubSubClient::send(const string& message)
{
    bool succeed = false;
    if (conn_ && conn_->connected())
    {
        //向服务器发送指令
        conn_->send(message);
        succeed = true;
    }
    return succeed;
}
#endif