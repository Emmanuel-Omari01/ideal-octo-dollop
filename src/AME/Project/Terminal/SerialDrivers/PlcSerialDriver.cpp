/*
 * PlcSerialDriver.cpp
 *
 *  Created on: 06.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "PlcSerialDriver.h"

namespace AME_SRC {

PlcSerialDriver::PlcSerialDriver(SerialComInterface* newSciPtr): SerialDriver(newSciPtr) {}

PlcSerialDriver::~PlcSerialDriver() = default;

}  // namespace AME_SRC
