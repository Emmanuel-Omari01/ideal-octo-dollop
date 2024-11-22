/*
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_STORAGEMANAGEMENT_ISTORAGE_H_
#define SRC_AME_PROJECT_STORAGEMANAGEMENT_ISTORAGE_H_

#include <inttypes.h>

namespace AME_SRC {

class IStorage {
 public:
    enum storeStates {
        kStoreSuccess, kClearSuccess, kAddressInvalid, kFailure
    };
    virtual ~IStorage() {}

    virtual void store(const uint8_t *data, uint32_t address, uint16_t size) = 0;
    virtual void clear(uint32_t address, uint16_t size) = 0;
    virtual void setup() = 0;
    virtual bool isReady() = 0;
    virtual IStorage::storeStates getStatus() = 0;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_STORAGEMANAGEMENT_ISTORAGE_H_
