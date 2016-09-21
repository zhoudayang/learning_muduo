cmake_minimum_required(VERSION 3.5)
project(hub)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
find_package(Threads)
#build to get execuable file hub
add_executable(hub hub.cpp codec.cpp)
target_link_libraries(hub muduo_inspect muduo_base muduo_net ${CMAKE_THREAD_LIBS_INIT})

add_library(muduo_pubsub pubsub.h codec.cpp)
target_link_libraries(muduo_pubsub muduo_net muduo_base ${CMAKE_THREAD_LIBS_INIT})

add_executable(pub pub.cpp)
target_link_libraries(pub muduo_pubsub)

add_executable(sub sub.cpp)
target_link_libraries(sub muduo_pubsub)
