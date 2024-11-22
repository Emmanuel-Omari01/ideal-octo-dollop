/*
 * InterruptController.cpp
 *
 *  Created on: Jun 5, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "InterruptController.h"
#include <stdint.h>
#include "../SystemBase/Global2.h"
#include "../Terminal/Sci/SerialComInterface.h"
#include "../SystemBase/Driver/CompareTimer0.h"
#include "../SystemBase/MicrocontrollerConfig.h"
#include "../SignalProcessing/I2C/I2cApp.h"

namespace AME_SRC {

InterruptController::eIntSource InterruptController::currentMask = none;


InterruptController::InterruptController() {
    // TODO(AME) Auto-generated constructor stub
}

InterruptController::~InterruptController() {
    // TODO(AME) Auto-generated destructor stub
}

// Short: enable the desired interrupt(s) so an triggered interrupt can be handled
// Input:
//  mask - or masked bits to specify the interrupts to be enabled
//         select mask "all" to enable all interrupts
void InterruptController::enable(eIntSource mask) {
    uint32_t checkMask = 1;
    uint32_t internMask = (uint32_t) mask;
    uint32_t curMask = (uint32_t) currentMask;
    bool handled;

    while (internMask) {
        if (internMask & checkMask) {
            handled = true;
            switch (checkMask) {
                case plcModem:
                    SerialComInterface::getInstance(sci5)->enableInterrupts();
                    break;
                case terminal:
                    SerialComInterface::getInstance(sci12)->enableInterrupts();
                    break;
                case i2c:
                    i2c_app::i2cStartInterrupt();
                    break;
                case cmt0:
                    MicrocontrollerConfig::startCMT0();
                    break;
                case cmt1: {
                        CompareTimer1* cmtPtr = Global2::getCmt1();
                        cmtPtr->start();
                    }
                    break;
                default:
                    handled = false;
                    break;
            }
        }
        if (handled) {
            curMask |= checkMask;
            handled = false;
        }
        internMask &= ~checkMask;
        checkMask = checkMask << 1;
    }
    currentMask = (eIntSource) curMask;
}

// Short: disable the desired interrupt(s) so an triggered interrupt is NOT handled
// Input:
//  mask - or masked bits to specify the interrupts to be enabled
//         select mask "all" to enable all interrupts
void InterruptController::disable(eIntSource mask) {
    uint32_t checkMask = 1;
    uint32_t internMask = (uint32_t) mask;
    uint32_t curMask = (uint32_t) currentMask;
    bool handled;

    while (internMask) {
        if (internMask & checkMask) {
            handled = true;
            switch (checkMask) {
                case plcModem:
                    SerialComInterface::getInstance(sci5)->disableInterrupts();
                    break;
                case terminal:
                    SerialComInterface::getInstance(sci12)->disableInterrupts();
                    break;
                case i2c:
                    i2c_app::i2cStopInterrupt();
                    break;
                case cmt0:
                    MicrocontrollerConfig::stopCMT0();
                    break;
                case cmt1: {
                        CompareTimer1* cmtPtr = Global2::getCmt1();
                        cmtPtr->stop();
                    }
                    break;
                default:
                    handled = false;
                    break;
            }
        }
        if (handled) {
            curMask &= ~checkMask;
            handled = false;
        }
        internMask &= ~checkMask;
        checkMask = checkMask << 1;
    }
    currentMask = (eIntSource) curMask;
}

}  // namespace AME_SRC
