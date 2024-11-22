/*
 * Pin.h
 *
 *  Created on: 19.01.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_IOPIN_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_IOPIN_H_

#include "Pin.h"

namespace AME_SRC {

class IoPin: public Pin {
 public:
    IoPin(PortManager::portName newPortName,
            PortManager::pinPosition newPinPosition,
            PortManager::pinDirection newDirection);
    IoPin(PortManager::portName newPortName,
          PortManager::pinPosition newPinPosition,
          PortManager::pinDirection newDirection, pinLowActive newActiv);
    IoPin();

    virtual ~IoPin();
    virtual PortManager::pinLevel getLevel() const {
        return Pin::getLevel();
    }

    virtual void setLevel(PortManager::pinLevel pegel) {
        Pin::setLevel(pegel);
    }

    virtual PortManager::pinPosition getPosition() const {
        return Pin::getPosition();
    }

    void toogle();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_IOPIN_H_
