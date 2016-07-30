#include<iostream>
#include<unistd.h>
#include <thread>

using namespace std;

//同步计数器
class CountDownLatch {
public:
    explicit CountDownLatch(int count) : count_(count) { }

    void wait();

    void countDown();

private:
    int count_;
    //初始计数次数
    condition_variable cond_;
    mutable mutex mutex_;
};

//等待计数变为0
void CountDownLatch::wait() {
    unique_lock<mutex> lock(mutex_);
    while (count_ > 0)
        cond_.wait(lock);
}

//计数减1
void CountDownLatch::countDown() {
    unique_lock<mutex> lock(mutex_);
    --count_;
    if (count_ == 0)
        cond_.notify_all();
}

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
    sleep(1);
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