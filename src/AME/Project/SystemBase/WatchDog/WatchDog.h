/*
 * WatchDog.h
 *
 *  Created on: 03.02.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_WATCHDOG_WATCHDOG_H_
#define SRC_AME_PROJECT_SYSTEMBASE_WATCHDOG_WATCHDOG_H_

namespace AME_SRC {

class WatchDog {
 public:
    typedef enum {
        on, off
    } timerState;

    typedef enum {
        CLOCK_DIV1 = (0x0000U),
        CLOCK_DIV16 = (0x0020U),
        CLOCK_DIV32 = (0x0030U),
        CLOCK_DIV64 = (0x0040U),
        CLOCK_DIV128 = (0x00F0U),
        CLOCK_DIV256 = (0x0050U),
    } clockDivisionRatio;  // higher Divisor => longer reaction time

    typedef enum {
        TIMEOUT_128 = (0x0000U),
        TIMEOUT_512 = (0x0001U),
        TIMEOUT_1024 = (0x0002U),
        TIMEOUT_2048 = (0x0003U),
    } timeoutCycle;

    typedef enum {
        WINDOW_START_25 = (0x0000U),
        WINDOW_START_50 = (0x1000U),
        WINDOW_START_75 = (0x2000U),
        WINDOW_START_100 = (0x3000U),
    } windowStartPosition;

    typedef enum {
        WINDOW_END_75 = (0x0000U),
        WINDOW_END_50 = (0x0100U),
        WINDOW_END_25 = (0x0200U),
        WINDOW_END_0 = (0x0300U),
    } windowEndPosition;

    typedef enum {
        ResetIntterupt_Enabled = (0x00U), ResetIntterupt_Disabled = (0x80U)
    } resetInterruptSettings;

    typedef enum {
        countStopDisabled = (0x00U), countStopDuringSleepmode = (0x80U)
    } sleepModeCount;

    typedef struct {
        clockDivisionRatio divRatio;
        timeoutCycle timeOut;
        windowStartPosition startPosi;
        windowEndPosition endPosi;
        resetInterruptSettings interruptState;
        sleepModeCount countMode;
    } profile;

    static profile defaultProfile;
    WatchDog();
    explicit WatchDog(profile newProfile);
    WatchDog(clockDivisionRatio divRatio, timeoutCycle timeOut,
            windowStartPosition startPosi, windowEndPosition endPosi,
            resetInterruptSettings interruptState, sleepModeCount countMode);
    static void feed();

    virtual ~WatchDog();

    static bool isRestartNotTriggert() {
        return !restartFlag;
    }

    static void setRestartFlag(bool flag) {
        restartFlag = flag;
    }

 private:
    static void setTimer(timerState state);
    static void resetTimeOutCounter();
    static bool restartFlag;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_WATCHDOG_WATCHDOG_H_
