cmake_minimum_required(VERSION 3.5)
project(filetransfer)
find_package(Threads)
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
set(SOURCE_FILES1 version1.cpp)
add_executable(filetransfer1 ${SOURCE_FILES1})
add_executable(filetransfer2 version2.cpp)
add_executable(filetransfer3 version3.cpp)
target_link_libraries(filetransfer1 muduo_net muduo_base ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(filetransfer2 muduo_net muduo_base ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(filetransfer3 muduo_net muduo_base ${CMAKE_THREAD_LIBS_INIT})
