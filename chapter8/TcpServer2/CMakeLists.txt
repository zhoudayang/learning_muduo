cmake_minimum_required(VERSION 2.8)
project(TcpServer2)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package(Threads)
add_subdirectory(thread)
add_subdirectory(datetime)
add_subdirectory(logging)
set(lib_src
        Callbacks.h
        EventLoop.cpp
        Channel.cpp
        Poller.cpp
        Timer.h
        TimerId.h
        Timer.cpp
        TimerQueue.cpp
        InetAddress.cpp
        Socket.cpp
        SocketsOps.cpp
        Acceptor.cpp
        TcpConnection.cpp
        TcpServer.cpp
        )
add_library(event SHARED ${lib_src})
target_link_libraries(event time thread log)
add_executable(test8 test8.cpp)
target_link_libraries(test8 event  ${CMAKE_THREAD_LIBS_INIT})
