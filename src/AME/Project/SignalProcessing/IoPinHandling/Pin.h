/*
 * Pin.h
 *
 *  Created on: 05.04.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_PIN_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_PIN_H_

#include "PortManager.h"

namespace AME_SRC {

class Pin {
 public:
    typedef enum { inactive, active } pinLowActive;

 protected:
    PortManager::portName portName;
    PortManager::pinPosition position;
    PortManager::pinDirection direction;
    PortManager::pinMode functionMode;
    PortManager::pinLevel level;
    pinLowActive lowActive;
    PortManager *mangerPtr;

 public:
    Pin() :
        portName(PortManager::NoPort), position(PortManager::NoPin), direction(
                    PortManager::Input), functionMode(
                    PortManager::GeneralIOPort), level(PortManager::Low), lowActive(
        inactive), mangerPtr(PortManager::getInstance()) {
    }
    Pin(PortManager::portName newPortName,
        PortManager::pinPosition newPinPosition,
        PortManager::pinDirection newDirection,
        PortManager::pinMode newMode, pinLowActive newActive);
    Pin(PortManager::portName newPortName,
            PortManager::pinPosition newPinPosition,
            PortManager::pinDirection newDirection,
            PortManager::pinMode newMode);
    virtual ~Pin();
    void setLowActive(pinLowActive newLowActive);

    PortManager::pinDirection getDirection() const {
        return direction;
    }

    void setDirection(PortManager::pinDirection pin_direction) {
        this->direction = pin_direction;
    }

    virtual PortManager::pinLevel getLevel() const {
        uint32_t pegel = (uint32_t) mangerPtr->getPinLevel(portName, position);
        pegel ^= (uint32_t) lowActive;
        return (PortManager::pinLevel) pegel;
    }

    virtual void setLevel(PortManager::pinLevel pegel) {
        uint32_t an_aus = (uint32_t) pegel;
        an_aus ^= (uint32_t) lowActive;
        this->level = (PortManager::pinLevel) an_aus;
        mangerPtr->setPinLevel(portName, position, this->level);
    }

    virtual PortManager::pinPosition getPosition() const {
        return position;
    }

    void setPosition(PortManager::pinPosition _position_) {
        this->position = _position_;
    }

    PortManager::portName getPortName() const {
        return portName;
    }

    PortManager::pinMode getFunctionMode() const {
        return functionMode;
    }

    void setFunctionMode(PortManager::pinMode _functionMode_) {
        this->functionMode = _functionMode_;
    }

    void setFunctionByte(uint8_t byte) {
        mangerPtr->setPinFunctionByte(portName, position, byte);
    }
    virtual pinLowActive getLowActive() const {
        return lowActive;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_PIN_H_
