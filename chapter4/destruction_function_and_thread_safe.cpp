#include<iostream>
#include<pthread.h>

using namespace std;

class calendar {
public:
    virtual bool isHoliday(string date) const = 0;

    virtual ~calendar() { }
};

class AmericanCalendar : public calendar {
public:
    virtual bool isHoliday(string date) const {
        //just for example
        return true;
    }

    ~AmericanCalendar() {
        printf("American destruction called!\n");
    }
};

class BritishCalendar : public calendar {
public:
    virtual bool isHoliday(string date) const {
        //just for example
        return true;
    }

    ~BritishCalendar() {
        printf("BritishCalendar destruction called!\n");
    }
};


//全局变量，用于在工厂方法中返回
BritishCalendar britishCalendar;
AmericanCalendar americanCalendar;

//工厂方法
calendar &getCalendar(const string &region) {
    if (region == "American")
        return americanCalendar;
    else
        return britishCalendar;
}


void *threadFunc1(void *parm) {
    string region1 = "American";
    string region2 = "British";
    for (int i = 0; i < 10; i++) {
        if (getCalendar(region1).isHoliday("nothing")) {
            printf("this day is American holiday! <- thread1\n");
        }
        if (getCalendar(region2).isHoliday("nothing")) {
            printf("this day is British holiday!  <- thread1\n");
        }
    }
    //结束任务，线程退出
    exit(1);
    //此处调用exit函数，会将全局变量americanCalendar和britishCalendar析构。
    //而在线程2中，还要调用上述两个全局变量，会造成程序崩溃退出
    //exit调用全局析构函数有输出为证：
    
    /*
     输出：
        American destruction called!
        BritishCalendar destruction called!
     程序崩溃退出：
        pure virtual method called
     */
    
}


void *threadFunc2(void *parm) {
    string region1 = "American";
    string region2 = "British";
    for (; ;) {
        if (getCalendar(region1).isHoliday("nothing")) {
            printf("this day is American holiday! <- thread2\n");
        }
        if (getCalendar(region2).isHoliday("nothing")) {
            printf("this day is British holiday!  <- thread2\n");
        }
    }
}

int main() {
    pthread_t threads[2];
    int rc = pthread_create(&threads[0], NULL, threadFunc1, NULL);
    if (rc) {
        printf("%d", rc);
        exit(-1);
    }
    rc = pthread_create(&threads[1], NULL, threadFunc2, NULL);
    if (rc) {
        printf("%d", rc);
        exit(-1);
    }
    pthread_exit(NULL);
}
