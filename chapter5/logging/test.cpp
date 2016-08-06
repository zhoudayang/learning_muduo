//
// Created by fit on 16-8-6.
//

#include "logging/logging.h"

#include<stdio.h>

int main(){
    LOG_TRACE << "trace";
    LOG_DEBUG << "debug";
    LOG_INFO << "Hello";
    LOG_WARN << "World";
    LOG_ERROR << "Error";

}