#include <iostream>
#include <thread>
#include <unistd.h>
using namespace std;

void test(){
    printf("hello world\n");

}
int main(){
    thread t1(test);
    t1.detach();
    printf("I am main function\n");
}