#include<iostream>
#include<thread>
#include<queue>
#include<assert.h>
#include<unistd.h>

using namespace std;
//使用条件变量实现的一个简单的阻塞队列

mutex mutex_;
std::deque<int> queue1;
condition_variable cond;

int dequeue() {
    unique_lock<mutex> lock(mutex_);
    while (queue1.empty()) {
        cond.wait(lock);
    }
    assert(!queue1.empty());
    int top = queue1.front();
    queue1.pop_front();
    return top;
}

void enqueue(int x) {
    unique_lock<mutex> lock(mutex_);
    queue1.push_back(x);
    cond.notify_one();
}

mutex print_lock;

//线程1中该函数每间隔3秒就向阻塞对列中插入10个数字
void function1() {
    while (true) {
        for (int i = 0; i < 10; i++) {
            {
                enqueue(i);
                lock_guard<mutex> lock(print_lock);
                cout << "enqueue " << i << endl;
            }

        }
        sleep(3);
    }
}
//线程2中该函数不断尝试从阻塞队列中取出元素
void function2() {
    while (true) {
        {
            lock_guard<mutex> lock(print_lock);
            cout<<"dequeue and get value ";
            cout << dequeue() << endl;
        }
    }
}

int main() {
    thread t1(function1);
    thread t2(function2);
    t1.join();
    t2.join();

    return 0;
}