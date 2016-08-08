#include<iostream>
#include<vector>
using namespace std;
//契合线程安全的观察者模型写法 -> 充分利用智能指针
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
    //请注意这里必须是引用对象
    mutex &mutex_;
};
//前置声明
class Observable;
class Observer {
public:
    Observer(Observable *obj) : obj(obj) { }
    void update() {
        cout << "get update information" << endl;
    }
    ~Observer(){
        cout<<"Observer destructor!\n";
    }
private:
    Observable *obj;
};
class Observable {
private:
    mutex mutex_;
    vector<weak_ptr<Observer>> observers_;
public:
    void register_(weak_ptr<Observer> x);
    void notifyObservers();

};
//注册观察者
void Observable::register_(weak_ptr<Observer> x) {
    MutexLockGuard lock(mutex_);
    observers_.push_back(x);
}
//通知观察者
void Observable::notifyObservers() {
    MutexLockGuard lock(mutex_);
    auto it = observers_.begin();
    while (it != observers_.end()) {
        //尝试提升,这一步是线程安全的
        shared_ptr<Observer> obj(it->lock());
        if (obj) {
            //提升成功
            obj->update();
            //没有race condition,因为obj在栈上,对象不可能在本作用域内销毁
            ++it;
        }
        else {
            //对象已经销毁,从容器中拿掉weak_ptr
            it = observers_.erase(it);
        }
    }
}
int main() {
    Observable *observable = new Observable();
    //此处必须存储这些Observer对象,否则这些对象在离开循环作用域之后将被销毁
    vector<shared_ptr<Observer>> Observers;
    for(int i =0;i<5;i++){
        shared_ptr <Observer >ptr (new Observer(observable));
        weak_ptr<Observer> weakPtr(ptr);
        observable->register_(weakPtr);
        Observers.push_back(ptr);
    }
    //通知观察者
    observable->notifyObservers();
    return 0;
}