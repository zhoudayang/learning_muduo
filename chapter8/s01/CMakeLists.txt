cmake_minimum_required(VERSION 3.5)
project(s02)
find_package(Threads)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_subdirectory(base)
set(SOURCE_FILE Channel.cpp EventLoop.cpp Poller.cpp)
add_library(event ${SOURCE_FILE})
target_link_libraries(event base ${CMAKE_THREAD_LIBS_INIT})
add_executable(test1 test1.cpp)
target_link_libraries(test1 base event ${CMAKE_THREAD_LIBS_INIT})
add_executable(test2 test2.cpp)
target_link_libraries(test2 base event ${CMAKE_THREAD_LIBS_INIT})
add_executable(test3 test3.cpp)
target_link_libraries(test3 base event ${CMAKE_THREAD_LIBS_INIT})



