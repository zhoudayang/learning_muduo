cmake_minimum_required(VERSION 3.5)
project(logging)
find_package(Threads)
set(src Atomic.h Thread.cpp)
add_library(thread ${src})
target_link_libraries(thread ${CMAKE_TREAD_LIBS_INIT})