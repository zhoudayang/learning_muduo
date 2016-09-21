cmake_minimum_required(VERSION 3.5)
project(time_wheel)
find_package(Threads)
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
add_executable(server echo.cpp server.cpp)
target_link_libraries(server muduo_net muduo_base ${CMAKE_THREAD_LIBS_INIT})
add_executable(client client.cpp)
target_link_libraries(client muduo_net muduo_base ${CMAKE_THREAD_LIBS_INIT})


