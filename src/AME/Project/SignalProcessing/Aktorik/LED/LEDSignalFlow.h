/*
 * LEDSignalFlow.h
 *
 *  Created on: Feb 13, 2024
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_AKTORIK_LED_LEDSIGNALFLOW_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_AKTORIK_LED_LEDSIGNALFLOW_H_

#include "../../IoPinHandling/IoPin.h"

namespace AME_SRC {

class LEDSignalFlow {
 public:
    LEDSignalFlow(IoPin *led1, IoPin *led2);
    virtual ~LEDSignalFlow();

    void processSignals();

    void setIsAliveSignaled(bool isAliveSignaled) {
        isAliveSignaled_ = isAliveSignaled;
    }

    enum eLEDMode {
        booting,
        noSoftware,
        searchNet,
        joinNet,
        connected,
        forceJoin,
        poll,
        aliveRunning
    };

    void setLedMode(enum eLEDMode ledMode) {
        ledMode_ = ledMode;
    }

 private:
    int16_t blinkIntervall_;
    IoPin *led1_;
    IoPin *led2_;
    enum eLEDMode ledMode_;
    bool isAliveSignaled_;

    void blinkNoSoftwareSignal();
    void blinkBootSignal();
    void blinkSearchNetworkSignal();
    void blinkJoiningSignal();
    void toggleLEDsSequential();
    void toggleLEDsAlternately();
    void toggleLEDsSynchronous();
    void blinkNetworkConnectedSignal();
    void blinkForceJoinSignal();
    void blinkPollSignal();
    void blinkAliveSignal();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_AKTORIK_LED_LEDSIGNALFLOW_H_
