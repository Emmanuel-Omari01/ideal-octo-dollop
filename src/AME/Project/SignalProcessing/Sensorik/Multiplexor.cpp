/*
 * Multiplexor.cpp
 *
 *  Created on: 09.07.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "Multiplexor.h"

namespace AME_SRC {

Multiplexor::Multiplexor(IoPin *mux0, IoPin *mux1, IoPin *mux2, IoPin *channel) :
        channelSwitch0(mux0), channelSwitch1(mux1), channelSwitch2(mux2), channelTransfer(channel),
        channelStatus(Multiplexor::noChannel) {
}

PortManager::pinLevel Multiplexor::getChannelLevel() {
    return channelTransfer->getLevel();
}

void Multiplexor::switchChannel(uint8_t channelIndex) {
    switch (channelIndex) {
    case channel0:
        channelSwitch0->setLevel(PortManager::Low);
        channelSwitch1->setLevel(PortManager::Low);
        channelSwitch2->setLevel(PortManager::Low);
        break;
    case channel1:
        channelSwitch0->setLevel(PortManager::High);
        channelSwitch1->setLevel(PortManager::Low);
        channelSwitch2->setLevel(PortManager::Low);
        break;
    case channel2:
        channelSwitch0->setLevel(PortManager::Low);
        channelSwitch1->setLevel(PortManager::High);
        channelSwitch2->setLevel(PortManager::Low);
        break;
    case channel3:
        channelSwitch0->setLevel(PortManager::High);
        channelSwitch1->setLevel(PortManager::High);
        channelSwitch2->setLevel(PortManager::Low);
        break;
    case channel4:
        channelSwitch0->setLevel(PortManager::Low);
        channelSwitch1->setLevel(PortManager::Low);
        channelSwitch2->setLevel(PortManager::High);
        break;
    case channel5:
        channelSwitch0->setLevel(PortManager::High);
        channelSwitch1->setLevel(PortManager::Low);
        channelSwitch2->setLevel(PortManager::High);
        break;
    case channel6:
        channelSwitch0->setLevel(PortManager::Low);
        channelSwitch1->setLevel(PortManager::High);
        channelSwitch2->setLevel(PortManager::High);
        break;
    case channel7:
        channelSwitch0->setLevel(PortManager::High);
        channelSwitch1->setLevel(PortManager::High);
        channelSwitch2->setLevel(PortManager::High);
        break;
    default:
        return;
    }
    channelStatus = (Multiplexor::muxChannel) channelIndex;
}

Multiplexor::muxChannel Multiplexor::getChannel() {
    return channelStatus;
}

Multiplexor::~Multiplexor() = default;

}  // namespace AME_SRC
