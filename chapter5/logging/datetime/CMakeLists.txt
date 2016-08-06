cmake_minimum_required(VERSION 3.5)
project(logging)
set(src Timestamp.cc)
add_library(time SHARED ${src})