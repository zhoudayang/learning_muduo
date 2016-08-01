cmake_minimum_required(VERSION 3.5)
project(TimeQueue)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES main.cpp Atomic.h Thread.cpp datetime/Timestamp.cc EventLoop.cpp Channel.cpp Poller.cpp TimerQueue.cpp)
add_executable(main ${SOURCE_FILES} ${CMAKE_THREAD_LIBS_INIT})