/*
 * AdditionalAnalogInputDriver.h
 *
 *  Created on: 18.11.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ADDITIONALANALOGINPUTDRIVER_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ADDITIONALANALOGINPUTDRIVER_H_

#include "../../SignalProcessing/AnalogPinHandling/AnalogPin.h"

namespace AME_SRC {

class AdditionalAnalogInputDriver {
 private:
    static AnalogPin cpu_Analog1;
    static AnalogPin cpu_Analog2;
    static AnalogPin cpu_Analog3;
    static AnalogPin cpu_Analog4;
 public:
    AdditionalAnalogInputDriver();
    char* getAnalogLevelByChannel();
    virtual ~AdditionalAnalogInputDriver();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ADDITIONALANALOGINPUTDRIVER_H_
