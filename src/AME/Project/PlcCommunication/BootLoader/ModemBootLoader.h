/*
 * ModemBootLoader.h
 *
 *  Created on: 01.02.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_BOOTLOADER_MODEMBOOTLOADER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_BOOTLOADER_MODEMBOOTLOADER_H_

#include "BootCommandManager.h"

#include <cstdint>

#include "../../Terminal/SerialDrivers/SerialDriver.h"
#include "../../SystemBase/Driver/CompareTimer1.h"

namespace AME_SRC {

class ModemBootLoader {
 public:
    ModemBootLoader(SerialDriver *driver,
                    BootCommandManager::Baudrate newFlashBaud,
                    BootCommandManager::Baudrate newPlcBaud);
    virtual ~ModemBootLoader();
    void reactOnRequest(BootCommandManager::CommandID command);
    void updateTransmissionBlockPtr();

    uint8_t getSgementNumber() const {
        return sgementNumber;
    }

    void setSgementNumber(uint8_t sgementNumber = 0) {
        this->sgementNumber = sgementNumber;
    }

    static uint32_t getBootMessageLength() {
        return headerMessageLength;
    }

    void setBootMessageLength(uint32_t bootMessageLength) {
        this->headerMessageLength = bootMessageLength;
    }

    bool isFirmenwareStatusUnkown() const {
        return firmenwareStatusUnknown;
    }

    bool isFirmenwareInStorage();

    static void setFirmenwareStatusUnkown(bool bootloaderFinished) {
        firmenwareStatusUnknown = bootloaderFinished;
    }

    BootCommandManager::Baudrate getPlcBaud() const {
        return plcBaud;
    }

    BootCommandManager::Baudrate getFlashBaud() const {
        return flashBaud;
    }

    bool isNotBusy() const {
        return !busyState;
    }
    static void startCompareMatchTimer() {
        if (blockFragmentCount != -2) {
            cmtPtr->start();
        }
    }

 private:
    ModemBootLoader();
    uint8_t* getFirmenwareFragment(SerialDriver *driver);

    uint8_t* answerRequest(BootCommandManager::CommandID request);
    void wait560MikrosecAfterTransmission(SerialDriver *driver,
            bool waitTransmission = true);
    bool isOldSegmentNumber(int currentSegNumber);
    static void interruptPrintNextTransmission();
    static bool firmenwareStatusUnknown;
    static const uint8_t *nextTransmissionBlockPtr;
    bool isNewSegmentBlock;
    uint8_t sgementNumber;
    static bool busyState;
    static bool timerStopped;
    static int blockFragmentCount;
    static uint32_t headerMessageLength;
    static uint32_t fragmentMessageLength;
    static SerialDriver *modemInterface;
    BootCommandManager::Baudrate flashBaud;
    BootCommandManager::Baudrate plcBaud;
    static CompareMatchTimeDriver *cmtPtr;
    typedef enum {
        firstRequest,
        firstBreak,
        segmentTransmissionStart,
        startBreakTimer,
        waitBreakTimer,
        segmentCondition,
        segmentEnd,
        idle
    } bootStates;
    static bootStates transmittState;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_BOOTLOADER_MODEMBOOTLOADER_H_
