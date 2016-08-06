cmake_minimum_required(VERSION 3.5)
project(logging)
set(src Atomic.h Thread.cpp)
add_library(thread ${src})