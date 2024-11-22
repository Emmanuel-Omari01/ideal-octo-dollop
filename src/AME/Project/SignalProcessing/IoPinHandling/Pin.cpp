/*
 * Pin.cpp
 *
 *  Created on: 05.04.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "Pin.h"

namespace AME_SRC {

Pin::Pin(PortManager::portName newPortName,
        PortManager::pinPosition newPinPosition,
        PortManager::pinDirection newDirection, PortManager::pinMode newMode) :
        portName(newPortName), position(newPinPosition), direction(
                newDirection), functionMode(newMode) {
    mangerPtr = PortManager::getInstance();
    mangerPtr->setPortSwitchingRegister(portName, position);
    mangerPtr->setPinDirection(portName, position, direction);
    mangerPtr->setPinMode(portName, position, newMode);
    level = mangerPtr->getPinLevel(portName, position);
  lowActive = inactive;
}

void Pin::setLowActive(pinLowActive newLowActive) {
  lowActive = newLowActive;
}

Pin::Pin(PortManager::portName newPortName,
         PortManager::pinPosition newPinPosition,
         PortManager::pinDirection newDirection, PortManager::pinMode newMode,
         pinLowActive newActive) :
    portName(newPortName), position(newPinPosition), direction(
                newDirection), functionMode(newMode), lowActive(newActive) {
    mangerPtr = PortManager::getInstance();
    mangerPtr->setPortSwitchingRegister(portName, position);
    mangerPtr->setPinDirection(portName, position, direction);
    mangerPtr->setPinMode(portName, position, newMode);
    level = mangerPtr->getPinLevel(portName, position);
  lowActive = newActive;
}

Pin::~Pin() = default;

}  // namespace AME_SRC
