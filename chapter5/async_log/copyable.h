//
// Created by fit on 16-8-23.
//

#ifndef COPYABLE_H
#define COPYABLE_H
namespace muduo {
    //a tag class emphasises the objects are copyable
    //the empty base class optimization applies
    //any derived class of copyable should be a value type
    class copyable {

    };
}
#endif
