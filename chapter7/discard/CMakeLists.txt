cmake_minimum_required(VERSION 3.5)
project(discard)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
set(SOURCE_FILES main.cpp)
add_executable(discard ${SOURCE_FILES})
target_link_libraries(discard muduo_base muduo_net ${CMAKE_THREAD_LIBS_INIT} )
