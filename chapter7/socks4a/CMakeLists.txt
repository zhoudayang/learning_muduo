cmake_minimum_required(VERSION 3.5)
project(socks4a)
find_package(Threads)
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)

add_executable(socks4a socks4a.cpp tunnel.h)
target_link_libraries(socks4a muduo_net muduo_base ${CMAKE_THREAD_LIBS_INIT})