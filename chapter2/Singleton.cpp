#include<iostream>
#include <boost/core/noncopyable.hpp>
#include <thread>
using namespace std;
//线程安全的Singleton
template<typename T>
class Singleton {
public:
    static T &instance() {
        //保证只初始化一次
        pthread_once(&pthread_once_, &Singleton::init);
        return *value_;
    }
private:
    Singleton();
    ~Singleton();
    static void init() {
        value_ = new T();
    }
private:
    static pthread_once_t pthread_once_;
    static T *value_;
};
//指定静态对象的初始值
template<typename T>
pthread_once_t Singleton<T>::pthread_once_ = PTHREAD_ONCE_INIT;
//init value_ with NULL
template<typename T>
T *Singleton<T>::value_ = NULL;
class test {
public:
    test() : name_("test") ,value_(rand()){ }
    string name() {
        return name_+to_string(value_);
    }
private:
    string name_;
    int value_;
};
int main() {

    thread t1([&]() -> void {
        auto it = Singleton<test>::instance();
        string name = it.name();
        printf("%s\n",name.c_str());
    });
    thread t2([&]() -> void {
        auto it = Singleton<test>::instance();
        string name = it.name();
        printf("%s\n",name.c_str());
    });
    thread t3([&]() -> void {
        auto it = Singleton<test>::instance();
        string name = it.name();
        printf("%s\n",name.c_str());
    });
    t1.join();
    t2.join();
    t3.join();
    return 0;
}