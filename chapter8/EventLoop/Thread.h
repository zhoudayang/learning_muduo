#ifndef THREAD_H
#define THREAD_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <assert.h>
#include <sys/syscall.h>
#include <boost/function.hpp>
#include "Atomic.h"

namespace muduo {
    namespace detail {
        //返回线程的pid值
        pid_t gettid();
    }
}
namespace muduo {
    namespace CurrentThread {

        void cacheTid();

        int tid();

    }

}

namespace muduo {
    class Thread : boost::noncopyable {
    public:
        typedef boost::function<void()> ThreadFunc;

        Thread(const ThreadFunc &func, const std::string &n);

        ~Thread() {
            if (started_ && !joined_) {
                pthread_detach(pthreadId_);
            }
        }

        void start();

        void join() {
            assert(started_);
            assert(!joined_);
            joined_ = true;
            pthread_join(pthreadId_, NULL);
        }


        bool started() const {
            return started_;
        }

        pid_t tid() const {
            return *tid_;
        }

        const std::string &name() const {
            return name_;
        };


    private:
        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        boost::shared_ptr<pid_t> tid_;
        ThreadFunc func_;
        std::string name_;


    };

}
#endif