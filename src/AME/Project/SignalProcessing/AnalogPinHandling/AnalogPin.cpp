/*
 * AnalogPin.cpp
 *
 *  Created on: 17.11.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "AnalogPin.h"

namespace AME_SRC {

#pragma GCC diagnostic ignored "-Wstack-usage="
AnalogPin::AnalogPin() {
    Pin();
    converterPtr = ADConverter::getInstance();
    blockingMode = true;
}

AnalogPin::AnalogPin(PortManager::portName newPortName,
        PortManager::pinPosition newPinPosition,
        PortManager::pinDirection newDirection) :
        Pin(newPortName, newPinPosition, newDirection,
                PortManager::PeriphalFunction), converterPtr(
                ADConverter::getInstance()) {
    converterPtr->enableConversionInGroupAForChannel((ADConverter::scanChannel) position);
    blockingMode = true;
}

AnalogPin::~AnalogPin() = default;

double AnalogPin::read() {
    converterPtr->startConversion();
    if (blockingMode) {
        converterPtr->waitForConversionCompleted();
    }
    return converterPtr->getDataFromChannel((ADConverter::scanChannel) position);;
}

}  // namespace AME_SRC
