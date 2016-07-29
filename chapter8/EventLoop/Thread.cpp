#include "Thread.h"

#include <boost/weak_ptr.hpp>

namespace muduo {
    namespace detail {
        pid_t gettid() {
            return static_cast<pid_t> (::syscall(SYS_gettid));
        }
    }
    namespace CurrentThread {
        __thread int t_cacheTid = 0;
        __thread const char *t_threadName = "unknown";

        void cacheTid() {
            t_cacheTid = detail::gettid();
        }

        int tid() {
            /*
                __builtin_expect() 是 GCC (version >= 2.96）提供给程序员使用的，目的是将“分支转移”的信息提供给编译器，这样编译器可以对代码进行优化，以减少指令跳转带来的性能下降。

                __builtin_expect((x),1) 表示 x 的值为真的可能性更大；
                __builtin_expect((x),0) 表示 x 的值为假的可能性更大。

                  也就是说，使用 likely() ，执行 if 后面的语句 的机会更大，使用 unlikely()，执行 else 后面的语句的机会更大。通过这种方式，编译器在编译过程中，会将可能性更大的代码紧跟着起面的代码，从而减少指令跳转带来的性能上的下降。
             */
            //此处t_cachedTid为0的可能性极小(事实上只有第一次调用tid()函数时，cachedTid的值才为0),建议编译器按照这个事实进行编译优化
            if (__builtin_expect(t_cacheTid == 0, 0)) {
                cacheTid();
            }
            return t_cacheTid;
        }


    }
    Thread::Thread(const ThreadFunc &func, const std::string &n) :
            started_(false), joined_(false), pthreadId_(0), tid_(new pid_t(0)), func_(func), name_(n) {

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
                //ptid不再管理任何对象
                ptid.reset();
                //将提升的shared_ptr 释放
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
    started_ = true;
    ThreadData *data = new ThreadData(func_, name_, tid_);
    if (pthread_create(&pthreadId_, NULL, &startThread, data)) {
        started_ = false;
        delete data;
        abort();
    }
}