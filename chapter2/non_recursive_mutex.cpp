#include<iostream>
#include<vector>

using namespace std;

class Foo {
public:
    string key;

    Foo(string key) : key(key) { }
};

mutex mutex_;
vector<Foo> foos;

void post(const Foo &f) {
    //stl中有MutexLockGuard的模板
    lock_guard<mutex> lock(mutex_);
    foos.push_back(f);
}

void traverse() {
    //stl中有MutexLockGuard的模板
    lock_guard<mutex> lock(mutex_);
    for (auto it = foos.begin(); it != foos.end(); it++) {
        Foo f(it->key);
        post(f);
        //此处post在traverse的基础上,再次对mutex_加锁,因为mutex是不可重入的,所以会造成死锁
        //此处如果改为可重入的锁,因为对数据foos进行了修改,迭代器失效,程序会崩溃
        //所以使用不可重入的锁可以帮助我们思考代码对锁的期求,及早发现错误
    }
}


int main() {
    for (int i = 0; i < 10; i++) {
        post(to_string(i));//构造器的隐式变换,因为Foo接受string作为参数来构造Foo成员
    }
    traverse();
    //陷入死锁,程序不会终止

    return 0;
}