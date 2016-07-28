#pragma  once

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <assert.h>
#include <sys/syscall.h>
#include <boost/function.hpp>
#include "Atomic.h"

namespace muduo {
    namespace detail {
        //返回线程的pid值
        pid_t gettid() {
            return static_cast<pid_t > (::syscall(SYS_gettid));
        }
    }
}
namespace muduo {
    namespace CurrentThread {
        __thread int t_cacheTid = 0;

        void cacheTid() {
            t_cacheTid = detail::gettid();
        }

        inline int tid() {
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

}

namespace muduo {
    class Thread : boost::noncopyable {
    public:
        typedef boost::function<void()> ThreadFunc;

        Thread(const ThreadFunc &func, const std::string &n) :
                started_(false), joined_(false), pthreadId_(0), tid_(new pid_t(0)), func_(func), name_(n) {
            numCreated_.increment();
        }

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

        static int numCreated() {
            return numCreated_.get();
        }

    private:
        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        boost::shared_ptr<pid_t> tid_;
        ThreadFunc func_;
        std::string name_;

        static AtomicInt32 numCreated_;
    };

}