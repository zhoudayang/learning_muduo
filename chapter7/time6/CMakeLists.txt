cmake_minimum_required(VERSION 3.5)
project(time6)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
add_executable(time time.cpp)
target_link_libraries(time muduo_net muduo_base pthread)

