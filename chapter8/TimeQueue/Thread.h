#ifndef THREAD_H
#define THREAD_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <assert.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <boost/function.hpp>
#include "Atomic.h"
#include <pthread.h>

namespace muduo {
    namespace detail {
        //返回线程的pid值
        pid_t gettid();
    }
}
namespace muduo {
    namespace CurrentThread {

        //缓存pid_t
        void cacheTid();
        //获取pid_t
        int tid();

    }

}

namespace muduo {
    class Thread : boost::noncopyable {
    public:
        typedef boost::function<void()> ThreadFunc;

        Thread(const ThreadFunc &func, const std::string &n=std::string());
        ~Thread() {
            //分离线程，不再和此线程有任何联系
            if (started_ && !joined_) {
                pthread_detach(pthreadId_);
            }
        }

        void start();

        void join() {
            assert(started_);
            assert(!joined_);
            joined_ = true;
            //阻塞直至线程运行完成
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

        static int numCreated(){
            return numCreated_.get();
        }
    private:
        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        boost::shared_ptr<pid_t> tid_;
        ThreadFunc func_;
        std::string name_;
        //创建的线程数量
        static muduo::AtomicInt32 numCreated_;
    };

}
#endif
