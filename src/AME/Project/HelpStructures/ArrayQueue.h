/*
 * ArrayQueue.h
 *
 * Copyright (C) 2024 Tobias Hirsch - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the XYZ license.
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_ARRAYQUEUE_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_ARRAYQUEUE_H_

namespace AME_SRC {

template<typename T, int N>

class ArrayQueue {
 public:
    ArrayQueue() :
            front(0), rear(-1), size(0) {
        // Initialize the cue with the given size N
    }

    void enqueue(T value) {
        if (size < N) {
            rear = (rear + 1) % N;
            queue[rear] = value;
            size++;
        } else {
            // Queue is full
            // Here you can add error handling
        }
    }

    T dequeue() {
        if (!isEmpty()) {
            T item = queue[front];
            front = (front + 1) % N;
            size--;
            return item;
        } else {
            // Queue is empty
            // Here you can add error handling
            return T();  // Dummy Return Value
        }
    }

    bool isEmpty() const {
        return size == 0;
    }

 private:
    T queue[N];
    int front;
    int rear;
    int size;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_ARRAYQUEUE_H_
