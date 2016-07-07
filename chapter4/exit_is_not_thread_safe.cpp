#include<iostream>
#include <mutex>

using namespace std;

void someFunctionMayCallExit() {
    exit(1);
}

class GlobalObject {
public:
    void doit() {
        lock_guard<mutex> lock(mutex_);
        someFunctionMayCallExit();
    }

    ~GlobalObject() {
        printf("GlobalObject:~GlobalObject\n");
        lock_guard<mutex> lock(mutex_);
        //clean up
        printf("GloabalObject:~GlobalObject cleanning \n");
    }

private:
    mutex mutex_;
};

GlobalObject g_obj;

int main() {
    g_obj.doit();
    /*
        g_obj在调用方法doit的时候，触发了函数exit,引起了g_obj对象的析构。
        而g_obj的析构函数会试图加锁mutex_,此时mutex_已经被GlobalObject::doit()锁住了，于是就造成了死锁。

        因而，exit在C++中并不是线程安全的
     */

}