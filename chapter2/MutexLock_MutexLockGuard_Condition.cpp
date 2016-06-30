#include <iostream>
#include <boost/core/noncopyable.hpp>
#include <assert.h>
#include <sys/syscall.h>
#include <thread>

using namespace std;

namespace CurrentThread {
    //存放当前线程的pid_t
    __thread int t_cachedTid = 0;

    //返回当前线程的pid_t
    inline int tid();
}
namespace detail {

    pid_t gettid() {
        //调用系统函数,得到当前线程的tid
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }
}
namespace CurrentThread {
    extern __thread int t_cachedTid;

    //缓存tid
    void cacheTid() {
        t_cachedTid = detail::gettid();
    }

    //返回tid
    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }


}


class MutexLock {
public:
    MutexLock() : holder_(0) {
        //初始化mutex_,使用默认的锁属性
        pthread_mutex_init(&mutex_, NULL);
    }

    ~MutexLock() {
        assert(holder_ == 0);
        //销毁 mutex_
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockedByThisThread() {
        return holder_ == CurrentThread::tid();
    }

    void assertLocked() {
        assert(isLockedByThisThread());
    }

    //上锁
    void lock() {
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
    }

    //解锁
    void unlock() {
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }

    //返回mutex_地址
    pthread_mutex_t *getPthreadMutex() {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};

class MutexLockGuard : boost::noncopyable {
public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    MutexLock &mutex_;
};
//定义宏,用于防止下述错误:
/*
     void doit(){
        MutexLockGuard (mutex);//遗漏变量名,产生一个临时对象又马上销毁了
        //结果没有锁住临界区
        //正确的写法:
        //MutexLockGuard lock(mutex);
        //进入临界区

     }
 */
#define MutexLockGuard(x) static_assert(false,"missing mutex guard var name")

//封装后的条件变量
class Condition : boost::noncopyable {
public:
    explicit Condition(MutexLock &mutex) : mutex_(mutex) {
        pthread_cond_init(&pcond_, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&pcond_);
    }

    //wait
    void wait() {
        pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
    }

    //notifyOne
    void notify() {
        pthread_cond_signal(&pcond_);
    }

    //notifyAll
    void notifyAll() {
        pthread_cond_broadcast(&pcond_);
    }

private:
    MutexLock &mutex_;
    pthread_cond_t pcond_;

};

class CountDownLatch {
public:
    CountDownLatch(int count) : mutex_(), condition_(mutex_), count_(count) {

    }

    void wait() {
        MutexLockGuard lock(mutex_);
        while (count_ > 0)
            condition_.wait();
    }

    void countDown() {
        MutexLockGuard lock(mutex_);
        --count_;
        if (count_ == 0)
            condition_.notifyAll();
    }

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;

};

//全局变量
//初始计数器值为2
CountDownLatch test(2);
//输出锁
mutex print_lock;

void worker1() {
    {
        lock_guard<mutex> lock(print_lock);
        cout << "worker 1 begin to work" << endl;
    }
    sleep(2);
    test.countDown();
    {
        lock_guard<mutex> lock(print_lock);
        cout << "worker 1 complete" << endl;
    }
}


void worker2() {
    {
        lock_guard<mutex> lock(print_lock);
        cout << "worker 2 begin to work" << endl;
    }
    test.countDown();
    sleep(2);
    test.countDown();
    {
        lock_guard<mutex> lock(print_lock);
        cout << "worker 2 complete" << endl;
    }
}

void boss() {
    test.wait();
    {
        lock_guard<mutex> lock(print_lock);
        cout << "worker complete work, now I need to check!" << endl;
    }
}

//程序案例:工人1和工人2需要完成2件事情,完成之后由boss进行检查
int main() {
    thread t1(worker1);
    thread t2(worker2);
    thread t3(boss);
    t1.join();
    t2.join();
    t3.join();


    return 0;
}