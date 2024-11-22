/*
 * Node.h
 *
 *  Created on: 12.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_NODE_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_NODE_H_

template<class T> class Node {
 private:
    T data;       // the object information
    Node *next_;  // pointer to the next node element

 public:
    Node() :
            next_(0) {
    }

    explicit Node(T *newData) :
            next_(0) {
        data = T(*newData);
    }

    // Methods omitted for brevity
    T& getData() {
        return data;
    }
    T getDataCopy() {
        return data;
    }
    void setData(T _data_) {
        this->data = _data_;
    }

    void setDataReff(T &_data_) {
        this->data = _data_;
    }

    Node* getNext() {
        return next_;
    }

    void setNext(Node *next) {
        this->next_ = next;
    }

    void setNextNull() {
        (this->next_) = 0;
    }
};

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_NODE_H_
