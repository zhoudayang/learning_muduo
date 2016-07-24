#include <muduo/net/EventLoop.h>

#include <iostream>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

using namespace std;
using namespace muduo::net;

class Printer : boost::noncopyable {
public:
    Printer(EventLoop *loop) : loop_(loop), count_(0) {

    }

    void print() {
        if (count_ < 5) {
            cout << count_ << "\n";
            ++count_;
            //
            loop_->runAfter(1, boost::bind(&Printer::print, this));
        }
        else
            loop_->quit();
    }

    ~Printer() {
        cout << "final count is " << count_ << endl;
    }

private:
    EventLoop *loop_;
    int count_;

};

int main() {
    EventLoop loop;
    Printer printer(&loop);
    loop.loop();
}
