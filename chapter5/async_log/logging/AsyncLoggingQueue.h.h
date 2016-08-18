//
// Created by fit on 16-8-18.
//

#ifndef ASYNCLOGGINGQUEUE_H_H
#define ASYNCLOGGINGQUEUE_H_H

#include "LogFile.h"
#include "../thread/BlockingQueue.h"
#include "../thread/BoundedBlockingQueue.h"
#include "../thread/CountDownLatch.h"
#include "../thread/Thread.h"

#include <string>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace muduo {
    template<typename MSG, template<typename> class QUEUE>
    class AsyncLoggingT : boost::noncopyable {
    public:
        AsyncLoggingT(const string &basename, size_t rollSize) :
                running_(false),
                basename_(basename),
                rollSize_(rollSize),
                thread_(boost::bind(&AsyncLoggingT::threadFunc, this), "Logging"),
                latch_(1) {}

        AsyncLoggingT(const string &basename, size_t rollSize, int queueSize) :
                running_(false),
                basename_(basename),
                rollSize_(rollSize),
                thread_(boost::bind(&AsyncLoggingT::threadFunc, this), "Logging"),
                latch_(1),
                queue_(queueSize) {}

        ~AsyncLoggingT() {
            if (running_) {
                stop();
            }
        }

        void append(const char *logline, int len) {
            queue_.put(MSG(logline, len));
        }

        void start() {
            running_ = true;
            thread_.start();
            latch_.wait();
        }

        void stop() {
            running_ = false;
            queue_.put(MSG());
            thread_.join();
        }

    private:
        void threadFunc() {
            assert(running_ == true);
            latch_.countDown();
            LogFile output(basename_, rollSize_, false);
            time_t lastFlush = time(NULL);
            while (true) {
                MSG msg(queue_.take());
                if (msg.empty()) {
                    assert(running_ == false);
                    break;
                }
                output.append(mag.date(), msg.length());
            }
            output.flush();
        }

        bool running_;
        string basename_;
        size_t rollSize_;
        muduo::Thread thread_;
        muduo::CountDownLatch latch_;
        QUEUE<MSG> queue_;


    };
}
typedef AsyncLoggingT <string, muduo::BlockingQueue> AsyncLoggingUnboundedQueue;
typedef AsyncLoggingT <string, muduo::BoundedBlockingQueue> AsyncLoggingBoundedQueue;

struct LogMessage {
    LogMessage(const char *msg, int len) : length_(len) {
        assert(length_ <= sizeof data_);
        ::memcpy(data_, msg, length_);
    }

    LogMessage() : length_(0) {}

    LogMessage(const LogMessage &rhs) :
            length_(rhs.length_) {
        assert(length_ <= sizeof data_);
        ::memccpy(data_, rhs.data_, length_);
    }

    LogMessage &operator=(const LogMessage &rhs) {
        length_ = rhs.length_;
        assert(length_ <= sizeof data_);
    }

    const char *data() const {
        return data_;
    }

    int length() const {
        return length_;
    }

    bool empty() const {
        return length_ == 0;
    }

    char data_[4000];
    size_t length_;
};

typedef AsyncLoggingT <LogMessage, muduo::BlockingQueue> AsyncLoggingUnboundedQueueL;
typedef AsyncLoggingT <LogMessage, muduo::BoundedBlockingQueue> AsyncLoggingBoundedQueuel;


#endif //
