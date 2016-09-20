cmake_minimum_required(VERSION 3.5)
project(protobuf_codec)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
include_directories(/home/zhouyang/Documents/muduo/include)
link_directories(/home/zhouyang/Documents/muduo/lib)
include_directories(${PROTOBUF_INCLUDE_DIR})

add_library(protobuf_codec codec.cpp)
target_link_libraries(protobuf_codec  muduo_net  muduo_base ${PROTOBUF_LIBRARY} z )

add_library(query_proto query.pb.cc)
target_link_libraries(query_proto ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

add_executable(codec_test codec_test.cpp)
target_link_libraries(codec_test protobuf_codec query_proto)

add_executable(dispatcher_test dispatcher_test.cpp)
target_link_libraries(dispatcher_test query_proto muduo_net muduo_base)


add_executable(server server.cpp)
target_link_libraries(server protobuf_codec query_proto)

add_executable(client client.cpp)
target_link_libraries(client protobuf_codec query_proto)
