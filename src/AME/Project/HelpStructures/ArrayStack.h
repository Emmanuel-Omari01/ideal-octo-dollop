/*
 * ArrayStack.h
 *
 *  Created on: 30.05.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_ARRAYSTACK_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_ARRAYSTACK_H_

namespace AME_SRC {

class ArrayStack {
 public:
    static const int kSize = 10;
    ArrayStack();
    void push(int value);
    int pop();
    bool isEmpty() const {
        return top == -1;
    }
    virtual ~ArrayStack();
 private:
    int stack[kSize]{};
    int top;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_ARRAYSTACK_H_
