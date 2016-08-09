#include <deque>
#include <boost/noncopyable.hpp>
#include <thread>
#include <condition_variable>
#include <assert.h>
#include <unistd.h>

template <typename T>
class BlockingQueue:boost::noncopyable {
public:
    T dequeue(){
        std::unique_lock<std::mutex> lock(mutex_);
        while(queue.empty())
            cond_.wait(lock);
        assert(!queue.empty());
        T top = queue.front();
        queue.pop_front();
        return top;
    }
    void enqueue(T x){
        std::unique_lock<std::mutex> lock(mutex_);
        queue.push_back(x);
        cond_.notify_one();
    }
private:
    std::mutex mutex_;
    std::condition_variable cond_;
    std::deque <T> queue;
};


void enqueue(BlockingQueue<int> &queue){
    while(true){
        queue.enqueue(rand());
        sleep(1);
    }
}

void dequeue(BlockingQueue<int> &queue){
    while(true){
        printf("get %d from queue now!\n",queue.dequeue());
    }
}
int main(){
    BlockingQueue<int> queue;
    std::thread t1(enqueue,std::ref(queue));
    std::thread t2(dequeue,std::ref(queue));
    t1.join();
    t2.join();

    return 0;
}