#include<iostream>
#include<string>
#include<map>
#include<assert.h>

using namespace std;
//创建该对象,加锁
//删除该对象,解锁
//利用对象及作用域简化调用步骤

class MutexLockGuard {
public:
    //此处必须为引用
    explicit MutexLockGuard(mutex &mutex_)
            : mutex_(mutex_) {
        mutex_.lock();
    }

    ~MutexLockGuard() {
        mutex_.unlock();
    }

private:
    //使用thread中自带的锁
    mutex &mutex_;
};

class Stock {
public:
    string key;

    Stock(string key) : key(key) { }

    friend bool operator==(const Stock &stock1, const Stock &stock2);

    ~Stock() {
        cout << "delete " << key << endl;
    }

};

bool operator==(const Stock &stock1, const Stock &stock2) {
    return stock1.key == stock2.key;
}

//允许从this变为shared_ptr
class StockFactory : public enable_shared_from_this<StockFactory> {
public:
    shared_ptr<Stock> get(const string &key) {
        shared_ptr<Stock> pStock;
        MutexLockGuard lock(mutex_);
        //此处必须为引用
        weak_ptr<Stock> &wkStock = stocks_[key];
        pStock = wkStock.lock();
        //尝试提升
        if (!pStock) {
            //绑定新的Stock对象
            //将参数weak_ptr<StockFactory>(shared_from_this() 绑定到函数weakDeleteCallback中的第一个参数
            pStock.reset(new Stock(key),
                         bind(&StockFactory::weakDeleteCallback, weak_ptr<StockFactory>(shared_from_this()),
                              placeholders::_1));
            //更新map中的value
            wkStock = pStock;
        }
        //返回shared_ptr
        return pStock;
    }

    ~StockFactory() {
        cout << "delete factory" << endl;
    }

private:
    //弱回调函数,保证factory对象的生命周期不因为绑定了bind函数而延长
    //!!!!!! important
    static void weakDeleteCallback(const weak_ptr<StockFactory> &wkFactory, Stock *stock) {
        //尝试提升StockFactory对象
        //需要判断factory对象是否有效
        shared_ptr<StockFactory> factory(wkFactory.lock());
        if (factory) {//factory对象健在,从map容器中清理该Stock
            factory->removeStock(stock);
        }
        //清理Stock
        delete stock;
    }

    //从map容器中删除对应股票
    void removeStock(Stock *stock) {
        if (stock) {
            MutexLockGuard lock(mutex_);
            stocks_.erase(stock->key);
        }
    }

    mutable mutex mutex_;
    map<string, weak_ptr<Stock>> stocks_;
};

//用于比较两个shared_ptr<Stock>类型数据是否相等
bool operator==(shared_ptr<Stock> stock1, shared_ptr<Stock> stock2) {
    return *stock1.get() == *stock2.get();
}

//factory在上层作用域
void testLongLifeFactory() {
    shared_ptr<StockFactory> factory(new StockFactory());
    {
        shared_ptr<Stock> stock1 = factory->get("IBM");
        shared_ptr<Stock> stock2 = factory->get("IBM");

        assert(stock1 == stock2);
        //stock destructs here
    }
    //factory destructs here

}

//factory在内部作用域中
void testShortLifeFactory() {
    shared_ptr<Stock> stock1;
    {
        shared_ptr<StockFactory> factory(new StockFactory());
        stock1 = factory->get("IBM");
        shared_ptr<Stock> stock2 = factory->get("IBM");
        assert(stock1 == stock2);
        //factory destructor here
    }
    //stock destructor here
}


int main() {
    testLongLifeFactory();
    testShortLifeFactory();

    /* print :
        delete IBM
        delete factory
        delete factory
        delete IBM
     */

    return 0;
}