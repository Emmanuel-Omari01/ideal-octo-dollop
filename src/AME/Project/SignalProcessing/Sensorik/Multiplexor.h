/*
 * Multiplexor.h
 *
 *  Created on: 09.07.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_SENSORIK_MULTIPLEXOR_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_SENSORIK_MULTIPLEXOR_H_

#include <cstdint>

#include "../IoPinHandling/IoPin.h"

namespace AME_SRC {

class Multiplexor {
 public:
    typedef enum {
        channel0,
        channel1,
        channel2,
        channel3,
        channel4,
        channel5,
        channel6,
        channel7,
        noChannel
    } muxChannel;
    Multiplexor(IoPin *mux0, IoPin *mux1, IoPin *mux2, IoPin *channel);
    void switchChannel(uint8_t channelIndex);
    PortManager::pinLevel getChannelLevel();
    muxChannel getChannel();
    virtual ~Multiplexor();

 private:
    muxChannel channelStatus;
    IoPin *channelSwitch0;
    IoPin *channelSwitch1;
    IoPin *channelSwitch2;
    IoPin *channelTransfer;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_SENSORIK_MULTIPLEXOR_H_
