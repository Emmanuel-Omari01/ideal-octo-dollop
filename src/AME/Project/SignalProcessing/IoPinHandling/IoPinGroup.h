/*
 * IoPinGroupe.h
 *
 *  Created on: 24.02.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_IOPINGROUP_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_IOPINGROUP_H_

#include "IoPin.h"

#include <cstdint>
#include <cstddef>

namespace AME_SRC {

class IoPinGroup {
 public:
    IoPinGroup();
    IoPinGroup(int ioCount, ...);
    void setLevelForBitMask(PortManager::pinLevel level, uint16_t bitMask);
    uint16_t get2ByteForLevel(PortManager::pinLevel level);
    uint8_t getPinAmount();
    IoPin* operator[](int index);
    virtual ~IoPinGroup();

 private:
    static const uint16_t groupMaxAmount = 16;
    IoPin *pinArray[groupMaxAmount];
    uint8_t pinAmount_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_IOPINGROUP_H_
