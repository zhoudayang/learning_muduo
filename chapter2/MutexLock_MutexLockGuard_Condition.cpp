#include <iostream>
#include <boost/core/noncopyable.hpp>
#include <assert.h>
#include <sys/syscall.h>
#include <thread>

using namespace std;

namespace detail {
    //返回线程的pid值
    pid_t gettid() {
        return static_cast<pid_t > (::syscall(SYS_gettid));
    }
}
namespace CurrentThrerad {
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
};


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