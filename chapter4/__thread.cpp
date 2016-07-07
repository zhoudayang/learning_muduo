#include<iostream>
#include <thread>
#include <vector>

using namespace std;

__thread int i;
/*
 __thread 是GCC内置的线程局部存储设施。它的效率比较高效,存取效率可与全局变量比肩。
 __thread 使用规则:只能修饰POD类型,不能修饰class类型,因为无法自动调用构造和析构函数。
 __thread 变量的初始化只能使用编译器常量。

 __thread 变量是每个线程都有一个独立实体,每个线程的变量值互不干扰,他还可以用来修饰那些值可能会变,带有全局性,但是又不值得用全局锁保护的变量。
如下所示:线程t1和t2中都引用了__thread类型的变量i,并在运行中不断进行修改,但是输出显示这两个线程并没有互相干扰。


 */
void threadFunc1(){
    for(i=0;i<10;i++){
        printf("%d\n",i);
    }
}
void threadFunc2(){
    for(i=-1;i>-10;i--)
        printf("%d\n",i);
}

int main() {
    thread t1(threadFunc1);
    thread t2(threadFunc2);
    t1.join();
    t2.join();
    //主线程中的i与上述两个线程也互不干扰,所以此处i为0
    cout<<"print from main ";
    cout<<i<<endl;
    return 0;
}