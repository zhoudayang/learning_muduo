#include<iostream>

using namespace std;


//创建该对象,加锁
//删除该对象,解锁
//利用对象及作用域简化调用步骤

class MutexLockGuard {
public:
    explicit MutexLockGuard(mutex &mutex_)
            : mutex_(mutex_) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    //使用thread中自带的锁
    mutex &mutex_;
};

//使用锁保护的Counter类，能够保证多线程下值的读写不受影响
class Counter {
public:
    Counter() : value_(0) { }

    int64_t value() const;

    int64_t getAndIncrease();

private:
    int64_t value_;
    //使用mutable关键字保证在const函数中也能修改该对象
    mutable mutex mutex_;
};

int64_t Counter::value() const {
    MutexLockGuard lock(mutex_);
    return value_;
}

int64_t Counter::getAndIncrease() {
    MutexLockGuard lock(mutex_);
    int64_t ret = value_++;
    return ret;
}

void *add_and_print(void *arg) {
    Counter *counter = static_cast<Counter *> (arg);
    cout << counter->getAndIncrease() << endl;
}

int main() {
    pthread_t tids[5];
    Counter *counter = new Counter();
    for (int i = 0; i < 5; i++) {
        int ret = pthread_create(&tids[i], NULL, add_and_print, static_cast<void *>(counter));
        if (ret != 0) {
            cout << "pthread_create error: error_code=" << ret << endl;
        }
    }
    //因为访问和修改值使用了锁,保证了Counter中的值value_自增了5次
    //所以虽然输出可能有先后,但是一定会输出4
    pthread_exit(NULL);

}
