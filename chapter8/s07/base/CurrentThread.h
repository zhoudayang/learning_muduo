#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include <stdint.h>

namespace muduo {
    namespace CurrentThread {
        //cache thread tid
        extern __thread int t_cachedTid;
        //thread tid name
        extern __thread char t_tidString[32];
        //the length of t_tidString
        extern __thread int t_tidStringLength;
        //thread name
        extern __thread const char *t_threadName;

        void cacheTid();

        inline int tid() {
            //t_cacheTid 为０的可能性极小，请编译器在此基础上进行编译优化
            if (__builtin_expect(t_cachedTid == 0, 0)) {
                //缓存　线程 id
                cacheTid();
            }
            return t_cachedTid;
        }

        inline const char *tidString() {
            return t_tidString;
        }

        inline int tidStringLength() {
            return t_tidStringLength;
        }

        inline const char *name() {
            return t_threadName;
        }

        //if is main thread? only if thread pid equals to process pid
        bool isMainThread();

        //sleep for usec 微秒
        void sleepUsec(int64_t usec);
    }


}


#endif
