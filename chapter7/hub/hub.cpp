//
// Created by zhouyang on 16-7-27.
//

#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <map>
#include <set>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

namespace pubsub {
    typedef std::set<string> ConnectionSubscription;

    class Topic : public muduo::copyable {
    public:
        Topic(const string &topic) : topic_(topic) { }

        void add(const TcpConnectionPtr &con) {
            audiences_.insert(con);
            if (lastPubTime_.valid()) {
                con->send(makeMessage());
            }
        }

        void remove(const TcpConnectionPtr &con) {
            audiences_.erase(con);
        }

        void publish(const string &content, Timestamp time) {
            content_ = content;
            lastPubTime_ = time;
            string message = makeMessage();
            for (std::set<TcpConnectionPtr>::iterator it = audiences_.begin(); it != audiences_.end(); it++) {
                (*it)->send(message);
            }
        }

    private:
        string makeMessage() {
            return "pub" + topic_ + "\r\n" + content_ + "\r\n";
        }

        string topic_;
        string content_;
        Timestamp lastPubTime_;
        std::set<TcpConnectionPtr> audiences_;

    };


    class PubSubServer : boost::noncopyable {
    public:
        PubSubServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr)
                : loop_(loop), server_(loop, listenAddr, "PubSubServer") {
            server_.setConnectionCallback();
            server_.setMessageCallback();
            loop->runEvery();
        }

        void start() {
            server_.start();
        }

    private:

        EventLoop *loop_;
        TcpServer server_;
        std::map<string, Topic> topics_;

    };

}