/*
 * IoPinGroupe.cpp
 *
 *  Created on: 24.02.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "IoPinGroup.h"

#include <cstdarg>

namespace AME_SRC {

IoPinGroup::IoPinGroup() {
  pinAmount_ = 0;
  for (int i = 0; i < groupMaxAmount; i++) {
    pinArray[i] = NULL;
  }
}

IoPinGroup::IoPinGroup(int ioCount, ...) {
  pinAmount_ = (uint8_t) ioCount;
    va_list valist;
    /* initialize valist for num number of arguments */
    va_start(valist, ioCount);

    /* access all the arguments assigned to valist */
    for (int i = 0; i < ioCount; i++) {
        pinArray[i] = va_arg(valist, IoPin*);
    }
    /* clean memory reserved for valist */
    va_end(valist);
}

void IoPinGroup::setLevelForBitMask(PortManager::pinLevel level,
        uint16_t bitMask) {
    for (int i = 0; i < pinAmount_; i++) {
        if (bitMask & (1 << i)) {
            pinArray[i]->setLevel(level);
        }
    }
}

uint16_t IoPinGroup::get2ByteForLevel(PortManager::pinLevel level) {
    uint32_t levelResult = 0;
    for (int i = 0; i < groupMaxAmount; i++) {
        if ((pinArray[i] != 0) && (pinArray[i]->getLevel() == level)) {
            levelResult |= 1 << i;
        }
    }
    return (uint16_t) levelResult;
}

uint8_t IoPinGroup::getPinAmount() {
    return pinAmount_;
}
IoPinGroup::~IoPinGroup() = default;

IoPin* IoPinGroup::operator[](int index) {
    if (index >= pinAmount_) {
        throw "Array index out of bound, exiting";
    }
    return pinArray[index];
}

}  // namespace AME_SRC
