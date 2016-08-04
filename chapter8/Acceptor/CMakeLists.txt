cmake_minimum_required(VERSION 3.5)
project(Acceptor)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(lib_src datetime/Timestamp.cc Atomic.h Callbacks.h Timer.cpp TimerId.h Thread.cpp EventLoop.cpp Poller.cpp Channel.cpp TimerQueue.cpp EventLoopThread.cpp)
#编译生成动态链接库
add_library(event SHARED ${lib_src})
