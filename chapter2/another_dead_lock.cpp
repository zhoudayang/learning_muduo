#include<iostream>
#include<set>
#include<unistd.h>
#include<thread>

using namespace std;
class Request;
class Inventory {
public:
    //向set中添加Request对象
    void add(Request *request) {
        lock_guard<mutex> lock(mutex_);
        requests_.insert(request);
    }
    //从set中删除Request对象
    void remove(Request *request) {
        lock_guard<mutex> lock(mutex_);
        requests_.erase(request);
    }
    void printAll() const;
private:
    mutable mutex mutex_;
    set<Request *> requests_;
};
//全局变量
Inventory inventory;
class Request {
public:
    Request(int key) : key(key) { }
    //向inventory中的set添加Request对象指针
    void process() {
        lock_guard<mutex> lock(mutex_);
        inventory.add(this);
    }
    //从inventory中的set删除Request对象指针
    ~Request() {
        lock_guard<mutex> lock(mutex_);
        sleep(1);
        inventory.remove(this);
    }
    //输出Request对象中的成员 key
    void print() const {
        lock_guard<mutex> lock(mutex_);
        cout << key << endl;
    }
private:
    int key;
    //需要在const成员函数中修改mutex_,所以类型应该为mutable
    mutable mutex mutex_;
};
//虽然有前向声明,但是没有Request对象中print函数的前向声明,所以需要将printAll函数定义在类体Request之后
/*
     You must have the definition of class B before you use the class. How else would the compiler otherwise know that there exists such a function as B::add?
    Either define class B before class A, or move the body of A::doSomething to after class B have been defined, like
    class B;
    class A
    {
        B* b;
        void doSomething();
    };
    class B
    {
        A* a;
        void add() {}
    };
    void A::doSomething()
    {
        //called function defined in class B
        b->add();
    }
 */
void Inventory::printAll() const {
    lock_guard<mutex> lock(mutex_);
    sleep(1);//为了容易复现死锁,这里使用了延时
    for (auto it = requests_.begin(); it != requests_.end(); it++) {
        (*it)->print();
    }
    cout << "Inventory::printAll() unlocked" << endl;
}
void threadFunc() {
    Request *request = new Request(1);
    request->process();
    ///Request 首先获取类Request中定义的锁,然后调用Inventory中的add或者remove方法,获取类Inventory中定义的锁
    delete request;
}

int main() {
    //向inventory中添加一条记录
    Request * request = new Request(10);
    //主线程首先获取类Inventory中的锁
    inventory.add(request);
    thread t1(threadFunc);
    usleep(500*1000);
    inventory.printAll();
    t1.join();
    /*
     *主线程是先调用Inventory::printAll再调用Request::print,
     *threadFunc线程是先调用Request::~Request再调用 Inventory::remove
     *这两个调用序列的加锁顺序正好相反,从而产生了死锁现象
     */
    //解决方法:把print移出printAll函数的临界区 或者把remove移出~Request函数的临界区(先remove,再加锁)
    return 0;
}