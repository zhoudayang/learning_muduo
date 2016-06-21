#include<iostream>

using namespace std;

class Request {
public:
    Request(int value) : value(value) { }

    void process() {
        lock_guard<mutex> lock(mutex_);
        value++;
        //调用print函数,同时对mutex_加锁两次,会造成死锁
        //print();
        //error!
        //此时应该调用print的已经加锁的版本
        printWithLockHold();

    }

    //在没有加锁的情况下调用这个函数
    void print() {
        lock_guard<mutex> lock(mutex_);
        cout << value << endl;
    }

    //在已经加锁的情况下调用这个函数
    void printWithLockHold() {
        cout << value << endl;
    }

private:
    mutable mutex mutex_;
    int value;
};

int main() {

    Request *request = new Request(10);
    request->process();
    
    return 0;
}