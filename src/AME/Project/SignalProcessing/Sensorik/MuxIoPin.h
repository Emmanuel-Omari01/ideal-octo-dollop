/*
 * MuxIoPin.h
 *
 *  Created on: 14.07.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_SENSORIK_MUXIOPIN_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_SENSORIK_MUXIOPIN_H_

#include "Multiplexor.h"

#include <cstdint>

namespace AME_SRC {

class MuxIoPin: public IoPin {
 private:
    Multiplexor *multiplexorRef;
    uint8_t channelID;
 public:
    MuxIoPin(Multiplexor *multiplexorPtr, uint8_t channelID);
    virtual ~MuxIoPin();
    PortManager::pinLevel getLevel() const {
        multiplexorRef->switchChannel(channelID);
        return multiplexorRef->getChannelLevel();
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_SENSORIK_MUXIOPIN_H_
