/*
 * Pin.cpp
 *
 *  Created on: 19.01.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "IoPin.h"

namespace AME_SRC {

IoPin::IoPin(PortManager::portName newPortName,
        PortManager::pinPosition newPinPosition,
        PortManager::pinDirection newDirection) :
        Pin(newPortName, newPinPosition, newDirection,
                PortManager::GeneralIOPort) {
}

IoPin::IoPin(PortManager::portName newPortName,
             PortManager::pinPosition newPinPosition,
             PortManager::pinDirection newDirection, pinLowActive newActiv) :
        Pin(newPortName, newPinPosition, newDirection,
                PortManager::GeneralIOPort, newActiv) {
}

IoPin::IoPin() :
        Pin() {
}

IoPin::~IoPin() = default;

void IoPin::toogle() {
    if (getLevel() == PortManager::High) {
        level = PortManager::Low;
    } else {
        level = PortManager::High;
    }
    setLevel(level);    // mangerPtr->setPinLevel(portName,position, level)
}

}  // namespace AME_SRC

