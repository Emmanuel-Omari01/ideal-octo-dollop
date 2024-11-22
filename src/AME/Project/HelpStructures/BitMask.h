/*
 * BitMask.h
 *
 *  Created on: 19.09.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_BITMASK_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_BITMASK_H_

#include <cstdint>

namespace AME_SRC {

template<typename T>
class BitMask {
 private:
    T mask;

 public:
    BitMask() {
        mask = 0;
    }

    bool isSet(int i) {
        return (mask & (1 << i));
    }

    void set(int i) {
        mask |= 1 << i;
    }

    void unSet(int i) {
        mask &= ~(1 << i);
    }

    void toggle(int i) {
        mask ^= mask << i + 1;
    }

    void clear() {  // compare with constructor
        mask = 0;
    }

    T getAllMask() {
        return mask;
    }
};

/***
 * Template for an modified class in compare to BitMask (Aim: Use memory sizes greater than 32 bits
 template<uint8_t>
 class BitMask<uint8_t> {
 private:
 long long mask;
 public:
 BitMask(){
 mask = 0;
 }

 bool isSet(int i){
 return (mask &(1<<i));
 }

 void set(int i){
 mask|= 1 << i;
 }

 void unSet(int i){
 mask&=~( 1<<i);
 }

 void toggle(int i){
 mask^=mask<<i+1;
 }

 };
 ***/

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_BITMASK_H_
