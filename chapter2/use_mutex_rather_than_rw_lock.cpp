#include<iostream>
#include<vector>
#include <map>
#include <memory>
#include <mutex>
#include <assert.h>

using namespace std;


//使用mutex替换读写锁的例子　
class CustomerData {
public:
    CustomerData() : data_(new Map) { };


    //查找
    int query(const string &customer, const string &stock) const;


private:
    typedef pair<string, int> Entry;
    typedef vector<Entry> EntryList;
    typedef map<string, EntryList> Map;
    typedef shared_ptr<Map> MapPtr;

    //更新
    void update(const string &customer, const EntryList &entries);

    //在entries中找到stock,因为entries已经按照stock进行了排序，此处可以使用二分查找　

    static int findEntry(const EntryList &entries, const string &stock) {
        //注意二分查找的实现　
        int l = 0;
        int r = entries.size() - 1;
        while (l <= r) {
            //不是直接(l+r)>>1,避免溢出
            int m = l + ((r - l) >> 1);
            if (entries[m].first > stock)
                r = m - 1;
            else if (entries[m].first < stock)
                l = m + 1;
            else
                return m;
        }
        return -1;
    }

    MapPtr getData() const {
        lock_guard<mutex> lock(mutex_);
        return data_;
    }


    mutable mutex mutex_;
    MapPtr data_;//Map的key是用户名，value是一个vector,里边存的是不同stock的最小交易间隔，vector已经排序，可以使用二分查找　
};

int CustomerData::query(const string &customer, const string &stock) const {
    MapPtr data = getData();
    //数据一旦拿到，就不再需要锁了
    auto entries = data->find(customer);
    if (entries != data->end())
        //调用类中的静态方法
        return findEntry(entries->second, stock);
    else
        return -1;
}

void CustomerData::update(const string &customer, const EntryList &entries) {
    lock_guard<mutex> lock(mutex_);
    if (!data_.unique()) {
        MapPtr newData(new Map(*data_));
        //改变智能指针管理的对象，更换成拷贝的成员
        data_.swap(newData);
    }
    //检查智能指针是否独占对象
    assert(data_.unique());
    //完成更新
    (*data_)[customer] = entries;

}

int main() {


    return 0;
}