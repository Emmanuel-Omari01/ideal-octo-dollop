/*
 * List.h
 *
 *  Created on: 12.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_LIST_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_LIST_H_

#include "Node.h"

#include <cstdlib>
#include <new>

template<typename T>
class List {
 private:
    Node<T> *head, *tail, *indexer;

 public:
    List();
    ~List();
    void add(T newData);
    void addReff(T &newData);
    void add(Node<T> &newDataNode);
    void clear() {
        head = NULL;
        tail = NULL;
        indexer = NULL;
    }

    bool isEmpty() {
        return head == NULL;
    }
    T* operator[](int index);
    List<T>& operator+(List<T> &list2);
    void resetIndex() {
        indexer = head;
    }

    Node<T>*& getHead() {
        return head;
    }

    void setHead(Node<T> &newHead) {
        head = &newHead;
    }

    const Node<T>*& getTail() const {
        return tail;
    }
    inline void loopIndexer() {
        if (indexer == NULL) {
            indexer = head;
        } else {
            indexer = indexer->getNext();
        }
    }

    Node<T>*& getIndexer() {
        return indexer;
    }
};

template<class T>
inline T* List<T>::operator[](int index) {
    indexer = head;
    for (int i = 0; i < index; i++) {
        if (indexer->getNext() == NULL) {
            return NULL;
        }
        indexer = indexer->getNext();
    }
    return &(indexer->getData());
}

template<class T>
inline List<T>& List<T>::operator+(List<T> &list2) {
    this->tail->setNext(list2.head);
    this->tail = list2.tail;
    return *this;
}

template<class T>
inline List<T>::List() :
        head(0), tail(0), indexer(0) {
}

template<class T>
inline List<T>::~List() {
    indexer = head;
    while (indexer != 0) {
        Node<T> *next = indexer->getNext();
        delete indexer;
        indexer = next;
    }
}

template<class T>
inline void List<T>::add(T newData) {
    if (head == NULL) {   // if our list is currently empty
        head = new Node<T>();  // Create new node of type T
        head->setData(newData);
        tail = head;
        indexer = head;
    } else {  // if not empty add to the end and move the tail
        Node<T> *temp = new Node<T>;
        temp->setData(newData);
        temp->setNextNull();
        tail->setNext(temp);
        tail = tail->getNext();
    }
}

template<class T>
inline void List<T>::addReff(T &newData) {
    if (head == NULL) {  // if our list is currently empty
        head = new Node<T>();  // Create new node of type T
        head->setDataReff(newData);
        tail = head;
        indexer = head;
    } else {  // if not empty add to the end and move the tail
        Node<T> *temp = new Node<T>;
        temp->setDataReff(newData);
        temp->setNextNull();
        tail->setNext(temp);
        tail = tail->getNext();
    }
}

template<class T>
inline void List<T>::add(Node<T> &newDataNode) {
    if (head == NULL) {  // if our list is currently empty
        head = &newDataNode;  // Create new node of type T
        tail = head;
        indexer = head;
    } else {  // if not empty add to the end and move the tail
        newDataNode.setNextNull();
        tail->setNext(&newDataNode);
        tail = tail->getNext();
    }
}

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_LIST_H_
