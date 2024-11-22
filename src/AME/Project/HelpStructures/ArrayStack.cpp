/*
 * ArrayStack.cpp
 *
 *  Created on: 30.05.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "ArrayStack.h"

namespace AME_SRC {

ArrayStack::ArrayStack() : top(-1) {
    // TODO(AME) Auto-generated constructor stub
}

void ArrayStack::push(int value) {
    if (top < kSize - 1) {
        if (top < 0) {
            stack[0] = value;
            top = 0;
        } else {
            stack[top + 1] = value;
            top++;
        }
    } else {
        throw("Stack overflow !!!!\n");
    }
}

int ArrayStack::pop() {
    if (top >= 0) {
        int n = stack[top];
        top--;
        return n;
    } else {
        return -1;
    }
}

ArrayStack::~ArrayStack() = default;

}  // namespace AME_SRC
