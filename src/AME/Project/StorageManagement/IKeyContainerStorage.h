/*
 * keyValueStorageBehaviour.h
 *
 *  Created on: 22.02.2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_STORAGEMANAGEMENT_IKEYCONTAINERSTORAGE_H_
#define SRC_AME_PROJECT_STORAGEMANAGEMENT_IKEYCONTAINERSTORAGE_H_

typedef unsigned long size_t;

#include <cstring>

#include "../HelpStructures/Uint8Operations.h"

namespace AME_SRC {
class IKeyContainerStorage {  // @suppress("Class has a virtual method and non-virtual destructor")
 public:
    typedef struct {
        const uint8_t *data;
        uint8_t length;
    } container;
    typedef union {
        struct {
            uint8_t invalid :1;  // Bit 0: invalid Flag
        } bit;
        uint8_t all;
    } key_flags;
    typedef union {
        struct {
            uint8_t key;
            key_flags flag;
            uint8_t dummy[2];
        } simple_key;
        struct {
            uint8_t key;
            key_flags flag;
            uint16_t index;
        } extended_key;
        uint32_t all;
    } skey;

    bool storeKeyValPair(skey key, uint8_t val) {
        temp[0] = val;
        con.data = temp;
        con.length = 1;
        return storeKeyContainer(key, con);
    }

    bool storeKeyValPair(skey key, uint16_t val) {
        uint8Operations::insertUint16intoUint8Array(temp, val);
        con.data = temp;
        con.length = 2;
        return storeKeyContainer(key, con);
    }
    bool storeKeyValPair(skey key, uint32_t val) {
        uint8Operations::insertUint32intoUint8Array(temp, val);
        con.data = temp;
        con.length = 4;
        return storeKeyContainer(key, con);
    }
    bool storeKeyValPair(skey key, uint8_t *pdata, uint8_t len) {
        if (len > sizeof(temp))
            len = sizeof(temp);
        memcpy(temp, pdata, len);
        con.data = temp;
        con.length = len;
        return storeKeyContainer(key, con);
    }
    virtual bool storeKeyContainer(skey key, container val) = 0;

    uint8_t loadUint8WithKey(skey *key) {
        container tmp;
        loadContainerWithKey(key, &tmp);
        return tmp.data[0];
    }

    uint16_t loadUint16WithKey(skey *key) {
        container tmp;
        loadContainerWithKey(key, &tmp);
        return uint8Operations::sum2BytesFromLSB(tmp.data);
    }

    uint32_t loadUint32WithKey(skey *key) {
        container tmp;
        loadContainerWithKey(key, &tmp);
        return uint8Operations::sum4BytesFromLSB(tmp.data);
    }

    virtual void loadContainerWithKey(skey *key, container *cntnr) = 0;
    virtual bool compare2Container(container con1, container con2) = 0;
    virtual bool compare2Flags(skey key1, skey key2) = 0;
    virtual bool containerDeleted() = 0;
    virtual void releaseContainerDeleted() = 0;

 private:
    uint8_t temp[21];
    container con;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_STORAGEMANAGEMENT_IKEYCONTAINERSTORAGE_H_
