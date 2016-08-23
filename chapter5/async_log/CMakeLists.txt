cmake_minimum_required(VERSION 3.6)
project(async_log)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES Atomic.h Condition.cpp CurrentThread.h Mutex.h Types.h)
add_executable(async_log ${SOURCE_FILES})