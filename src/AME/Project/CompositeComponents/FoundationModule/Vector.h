/*
 * Vector.h
 *
 *  Created on: 25.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_VECTOR_H_
#define SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_VECTOR_H_

#include <cstdio>

namespace AME_SRC {

template<typename T>
class Vector {
 public:
    Vector() :
            capacity_(0), size_(0), data_(0) {
    }

    ~Vector() {
        delete[] data_;
    }

    void push_back(const T &value) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = value;
    }

    void reserve(size_t newCapacity) {
        if (newCapacity > capacity_) {
            T *newData = new T[newCapacity];
            for (size_t i = 0; i < size_; ++i) {
                newData[i] = data_[i];
            }
            delete[] data_;
            data_ = newData;
            capacity_ = newCapacity;
        }
    }

    size_t size() const {
        return size_;
    }

    const T& operator[](size_t index) const {
        return data_[index];
    }

    T& operator[](size_t index) {
        return data_[index];
    }

 private:
    size_t capacity_;
    size_t size_;
    T *data_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_VECTOR_H_
