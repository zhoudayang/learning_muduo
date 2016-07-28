#include "Thread.h"

#include <boost/weak_ptr.hpp>

namespace muduo {
    namespace CurrentThread {
        __thread const char *t_threadName = "unknown";
    }
}
namespace {
    struct ThreadData {
        typedef muduo::Thread::ThreadFunc ThreadFunc;
        ThreadFunc func_;
        std::string name_;
        boost::weak_ptr<pid_t> wkTid_;

        ThreadData(const ThreadFunc &func,
                   const std::string &name,
                   const boost::shared_ptr<pid_t> &tid)
                : func_(func),
                  name_(name),
                  wkTid_(tid) {}

        void runInThread() {
            pid_t tid = muduo::CurrentThread::tid();
            boost::shared_ptr<pid_t> ptid = wkTid_.lock();
            if (ptid) {
                *ptid = tid;
                ptid.reset();
            }
            muduo::CurrentThread::t_threadName = name_.c_str();
            func_();
            muduo::CurrentThread::t_threadName = "finish";
        }


    };
    void *startThread(void *obj) {
        ThreadData *data = static_cast<ThreadData *>(obj);
        data->runInThread();
        delete data;
        return NULL;
    }
}

using namespace muduo;

void Thread::start() {
    assert(!started_);
    started_=true;
    ThreadData * data = new ThreadData(func_,name_,tid_);
    if(pthread_create(&pthreadId_,NULL,&startThread,data))
    {
        started_=false;
        delete data;
        abort();
    }
}