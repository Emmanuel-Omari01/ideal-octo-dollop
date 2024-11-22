/*
 * PlcSerialDriver.h
 *
 *  Created on: 06.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SERIALDRIVERS_PLCSERIALDRIVER_H_
#define SRC_AME_PROJECT_TERMINAL_SERIALDRIVERS_PLCSERIALDRIVER_H_

#include "SerialDriver.h"


#include <cstdint>

namespace AME_SRC {

class PlcSerialDriver: SerialDriver {
 public:
    explicit PlcSerialDriver(SerialComInterface *newSciPtr);
    virtual ~PlcSerialDriver();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_SERIALDRIVERS_PLCSERIALDRIVER_H_
