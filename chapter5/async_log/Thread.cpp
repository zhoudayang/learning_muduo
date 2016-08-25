//
// Created by fit on 16-8-24.
//

#include "Thread.h"
#include "CurrentThread.h"
#include "Exception.h"
#include "Logging.h"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/weak_ptr.hpp>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace muduo{
    namespace CurrentThread{
        __thread int t_cachedTid = 0;
        __thread char t_tidString[32];
        __thread int t_tidStringLength = 6;
        __thread const char * t_threadName = "unknown";
        const bool sameType =boost::is_same<int,pid_t>::value;
        BOOST_STATIC_ASSERT(sameType);
    }
    namespace detail{
        pid_t gettid(){
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }
        void afterFork(){
            muduo::CurrentThread::t_cachedTid = 0;
            muduo::CurrentThread::t_threadName="main";
            CurrentThread::tid();
            // no need to call pthread_atfork(NULL,NULL,&afterFork);
        }
        class ThreadNameInitializer{
        public:
            ThreadNameInitializer(){
                muduo::CurrentThread::t_threadName="main";
                CurrentThread::tid();
                pthread_atfork(NULL,NULL,&afterFork);
            }
        };
        ThreadNameInitializer init;
        struct ThreadData{
            typedef muduo::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            string name_;
            boost::weak_ptr<pid_t> wkTid_;
            ThreadData(const ThreadFunc &func,const string &name,const boost::shared_ptr<pid_t> &tid):
                    func_(func),name_(name),wkTid_(tid){}
            void runInThread(){
                pid_t tid = muduo::CurrentThread::tid();
                boost::shared_ptr<pid_t> ptid = wkTid_.lock();
                if(ptid){
                    *ptid = tid;
                    ptid.reset();
                }
                muduo::CurrentThread::t_threadName= name_.empty()? "muduoThread":name_.c_str();
                //PR_SET_NAME :把参数arg2作为调用进程的名字
                ::prctl(PR_SET_NAME,muduo::CurrentThread::t_threadName);
                try{
                    func_();
                    muduo::CurrentThread::t_threadName = "finished";
                }catch (const Exception &ex){

                }
            }
        };


    }
}