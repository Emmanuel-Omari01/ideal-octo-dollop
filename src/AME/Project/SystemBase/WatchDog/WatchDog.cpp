/*
 * WatchDog.cpp
 *
 *  Created on: 03.02.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "WatchDog.h"

#include <platform.h>
#include "../MicrocontrollerConfig.h"

namespace AME_SRC {

WatchDog::profile WatchDog::defaultProfile = { CLOCK_DIV64, TIMEOUT_2048,
        WINDOW_START_100, WINDOW_END_0, ResetIntterupt_Disabled,
        countStopDisabled };
bool WatchDog::restartFlag = false;

WatchDog::WatchDog() : WatchDog(defaultProfile) {
    setRestartFlag(false);
}

void WatchDog::feed() {
    if (isRestartNotTriggert()) {
        resetTimeOutCounter();
    }
}

void WatchDog::resetTimeOutCounter() {
    IWDT.IWDTRR = 0x00U;
    IWDT.IWDTRR = 0xFFU;
}

void WatchDog::setTimer(timerState state) {
    MicrocontrollerConfig::disableWriteProtection();
    SYSTEM.ILOCOCR.BYTE = (uint16_t) state;
    MicrocontrollerConfig::enableWriteProtection();
}

WatchDog::WatchDog(clockDivisionRatio divRatio, timeoutCycle timeOut,
        windowStartPosition startPosi, windowEndPosition endPosi,
        resetInterruptSettings interruptState, sleepModeCount countMode) {
    MicrocontrollerConfig::disableWriteProtection();
    IWDT.IWDTCR.WORD = timeOut | divRatio | endPosi | startPosi;
    IWDT.IWDTRCR.BYTE = interruptState;
    IWDT.IWDTCSTPR.BYTE = countMode;
    setTimer(on);
    MicrocontrollerConfig::enableWriteProtection();
}

WatchDog::WatchDog(profile newProfile) :
        WatchDog(newProfile.divRatio, newProfile.timeOut, newProfile.startPosi,
                newProfile.endPosi, newProfile.interruptState,
                newProfile.countMode) {
}

WatchDog::~WatchDog() = default;

}  // namespace AME_SRC


