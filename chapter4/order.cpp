#include<iostream>
#include <thread>

using namespace std;

int main() {
    thread t1([]() -> void {
        printf("hello world from t1\n");

    });
    thread t2([]() -> void {
        printf("hello world from t2\n");
    });
    t1.join();
    t2.join();

    cout << "hello world from main function!" << endl;

}
/*
	虽然t1.join()在前，但是实际运行的时候，有可能线程t2输出在前，这表明线程运行的实际顺序实际上是不确定的。
*/
