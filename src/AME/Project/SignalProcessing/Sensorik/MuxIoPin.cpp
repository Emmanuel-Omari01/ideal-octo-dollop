/*
 * MuxIoPin.cpp
 *
 *  Created on: 14.07.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "MuxIoPin.h"

namespace AME_SRC {

MuxIoPin::MuxIoPin(Multiplexor *multiplexorPtr, uint8_t channelID) :
        multiplexorRef(multiplexorPtr), channelID(channelID) {
}

MuxIoPin::~MuxIoPin() = default;

}  // namespace AME_SRC
