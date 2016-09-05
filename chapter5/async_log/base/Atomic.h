//
// Created by fit on 16-8-19.
//

#ifndef ATOMIC_H
#define ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>

//atomic integer class based on gcc marco definition

namespace muduo {
    namespace detail {
        template<typename T>
        class AtomicIntegerT : boost::noncopyable {
        public:
            AtomicIntegerT() : value_(0) {}

            //if value_ equals to 0, swap value_ and  0 ,and return value_
            T get() {
                return __sync_val_compare_and_swap(&value_, 0, 0);
            }


            //fetch value_ and add x to value_
            T getAndAdd(T x) {
                return __sync_fetch_and_add(&value_, x);
            }

            //add x to value_ and fetch value_
            T addAndGet(T x) {
                return getAndAdd(x) + x;
            }

            //add 1 to value_ and fetch value_
            T incrementAndGet() {
                return addAndGet(1);
            }

            //minus 1 from value_ and fetcho value_
            T decrementAndGet() {
                return addAndGet(-1);
            }

            //add x to value_
            void add(T x) {
                getAndAdd(x);
            }

            //add 1 to value_
            void increment() {
                incrementAndGet();
            }

            //minus 1 from value_
            void decrement() {
                decrementAndGet();
            }

            //get value_ and set newValue to value_
            T getAndSet(T newValue) {
                return __sync_lock_test_and_set(&value_, newValue);
            }

        private:
            volatile T value_;
        };
    }
    //声明类型
    typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
    typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}
#endif
