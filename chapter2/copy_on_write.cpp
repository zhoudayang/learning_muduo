#include<iostream>
#include<vector>
#include <memory>
#include <mutex>
#include <assert.h>
#include <thread>

using namespace std;

class Foo {
public:
    Foo(string name) : name_(name) { }

    Foo(const char *name) : name_(name) { }

    string name() {
        return name_;
    }

private:
    string name_;
};


vector<Foo> *FooLists = new vector<Foo>{"123", "456", "789"};
//智能指针需要管理在堆区的成员，在栈区的成员的内存会被自动管理
//如果将栈区元素的指针交给智能指针管理，会导致多调用一次析构函数，造成段错误
shared_ptr<vector<Foo>> g_foos(FooLists);
mutex mutex_;
mutex print_lock;
//对于写端，如果引用计数的值为１，可以直接对对象进行修改

void post(const Foo &f) {
    {
        lock_guard<mutex> print_guard(print_lock);
        cout << "post" << endl;
    }
    lock_guard<mutex> lock(mutex_);
    if (!g_foos.unique()) {
        //否则复制一个完整的vector<Foo>对象，对该对象进行修改
        //同时让全局的智能指针接管这个对象　
        g_foos.reset(new vector<Foo>(*g_foos));
        {
            lock_guard<mutex> print_guard(print_lock);
            cout << "copy the whole list" << endl;
        }
    }
    //此时需要保证g_foos智能指针的引用计数为１　
    assert(g_foos.unique());
    g_foos->push_back(f);
}

/*
 在read端，在读之前将引用计数的值加１，读完之后减去１，这样可以
 保证在读的时候其引用计数的值大于１，可以阻止并发写
*/
void traverse() {
    shared_ptr<vector<Foo>> foos=g_foos;//增加引用计数
    {
        lock_guard<mutex> lock(mutex_);
        foos = g_foos;
        assert(!g_foos.unique());
    }
    //此处读操作可以在临界区之外　
    for (auto it = foos->begin(); it != foos->end(); it++) {
            cout << it->name() << endl;
    }
}

int main() {
    thread t1([&]() -> void {
        traverse();

    });

    thread t2([&]()->void{
        Foo f("test");
        post(f);
    });
    

    t1.join();
    t2.join();


    return 0;
}