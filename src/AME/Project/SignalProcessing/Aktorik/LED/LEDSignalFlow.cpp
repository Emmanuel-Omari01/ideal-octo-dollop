/*
 * LEDSignalFlow.cpp
 *
 *  Created on: Feb 13, 2024
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "LEDSignalFlow.h"

namespace AME_SRC {

LEDSignalFlow::LEDSignalFlow(IoPin *led1, IoPin *led2) :
        blinkIntervall_ { 0 }, led1_ { led1 }, led2_ { led2 },
        ledMode_ { booting }, isAliveSignaled_ { false } {
}

LEDSignalFlow::~LEDSignalFlow() = default;

/***
 * This method deals with the LED representation of the module
 * This includes the alive LED and the LED for PLC connection
 */
void LEDSignalFlow::processSignals() {
    enum {
        intervall50ms = 50
    };
    blinkIntervall_ = (int16_t) (blinkIntervall_ - intervall50ms);
    if (blinkIntervall_ > 0) {
        return;
    }
    switch (ledMode_) {
    case noSoftware:
        blinkNoSoftwareSignal();
        break;
    case booting:
        blinkBootSignal();
        break;
    case searchNet:
        blinkSearchNetworkSignal();
        break;
    case joinNet:
        blinkJoiningSignal();
        break;
    case connected:
        blinkNetworkConnectedSignal();
        break;
    case forceJoin:
        blinkForceJoinSignal();
        break;
    case poll:
        blinkPollSignal();
        break;
    case aliveRunning:
        blinkAliveSignal();
        break;
    }
}

void LEDSignalFlow::blinkNoSoftwareSignal() {
    blinkIntervall_ = 100;
    led1_->toogle();
    led2_->toogle();
}

void LEDSignalFlow::blinkBootSignal() {
    blinkIntervall_ = 500;
    toggleLEDsSequential();
}

void LEDSignalFlow::toggleLEDsSequential() {
    static bool isLEDSwitch = false;
    if (isLEDSwitch) {
        led1_->toogle();
    } else {
        led2_->toogle();
    }
    isLEDSwitch = !isLEDSwitch;
}

void LEDSignalFlow::blinkSearchNetworkSignal() {
    blinkIntervall_ = 1000;
    toggleLEDsAlternately();
}

void LEDSignalFlow::blinkJoiningSignal() {
    blinkIntervall_ = 200;
    toggleLEDsAlternately();
}

void LEDSignalFlow::toggleLEDsAlternately() {
    static bool isLEDSwitch = false;
    if (isLEDSwitch) {
        led1_->setLevel(PortManager::High);
        led2_->setLevel(PortManager::Low);
    } else {
        led1_->setLevel(PortManager::Low);
        led2_->setLevel(PortManager::High);
    }
    isLEDSwitch = !isLEDSwitch;
}

void LEDSignalFlow::toggleLEDsSynchronous() {
    static bool isLEDSwitch = false;
    if (isLEDSwitch) {
        led1_->setLevel(PortManager::High);
        led2_->setLevel(PortManager::High);
    } else {
        led1_->setLevel(PortManager::Low);
        led2_->setLevel(PortManager::Low);
    }
    isLEDSwitch = !isLEDSwitch;
}

void LEDSignalFlow::blinkNetworkConnectedSignal() {
    blinkIntervall_ = 500;
    led1_->toogle();
    led2_->setLevel(PortManager::High);
}

void LEDSignalFlow::blinkForceJoinSignal() {
    blinkIntervall_ = 100;
    led1_->toogle();
    led2_->setLevel(PortManager::Low);
}

void LEDSignalFlow::blinkPollSignal() {
    blinkIntervall_ = 100;
    toggleLEDsSynchronous();
}

void LEDSignalFlow::blinkAliveSignal() {
    if (isAliveSignaled_) {
        isAliveSignaled_ = false;
        led1_->setLevel(PortManager::Low);
        blinkIntervall_ = 200;
    } else {
        led1_->setLevel(PortManager::High);
        blinkIntervall_ = 0;
    }
}

}  // namespace AME_SRC
