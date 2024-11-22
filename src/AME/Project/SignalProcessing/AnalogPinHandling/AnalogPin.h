/*
 * AnalogPin.h
 *
 *  Created on: 17.11.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ANALOGPIN_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ANALOGPIN_H_

#include "../../SignalProcessing/AnalogPinHandling/ADConverter.h"
#include "../IoPinHandling/IoPin.h"

namespace AME_SRC {

class AnalogPin: public Pin {
 public:
    AnalogPin();
    AnalogPin(PortManager::portName newPortName,
            PortManager::pinPosition newPinPosition,
            PortManager::pinDirection newDirection);
    double read();
    virtual ~AnalogPin();

 private:
    ADConverter *converterPtr;
    bool blockingMode;          // true (default): wait for conversion completed
                                // conversion time is ~2µs
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ANALOGPIN_H_
