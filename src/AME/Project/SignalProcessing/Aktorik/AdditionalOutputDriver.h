/*
 * AdditionalOutputDriver.h
 *
 *  Created on: 09.07.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_AKTORIK_ADDITIONALOUTPUTDRIVER_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_AKTORIK_ADDITIONALOUTPUTDRIVER_H_

#include <cstdint>

#include "../IoPinHandling/IoPin.h"

namespace AME_SRC {

class AdditionalOutputDriver {
 public:
    AdditionalOutputDriver();
    virtual ~AdditionalOutputDriver();
    static char* callRelayToggle(char *commandArguments);
    static IoPin* getRelayPinByIndex(const uint8_t relayIndex);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_AKTORIK_ADDITIONALOUTPUTDRIVER_H_
