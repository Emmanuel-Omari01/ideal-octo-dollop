/*
 * TimedSerialDriver.h
 *
 *  Created on: 08.02.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_TIMEDSERIALDRIVER_H_
#define SRC_AME_PROJECT_TERMINAL_TIMEDSERIALDRIVER_H_

#include <cstdint>

#include "SerialDrivers/SerialDriver.h"

namespace AME_SRC {

class TimedSerialDriver: public SerialDriver {
 public:
    TimedSerialDriver();
    virtual ~TimedSerialDriver();
    virtual void putchar(uint8_t ch);
    virtual char* get_buffer();
    virtual int txd_free(void);
    virtual void printRaw(char *array, int length);
    virtual void printRawFast(char *array, int length);
    virtual bool isNotTrasnferingData();
    static uint32_t popCurrentStringLength();
    void setNextStringPaddingTime(uint8_t _stringPaddingTime_) {
        this->nextPaddingTime = _stringPaddingTime_;
        paddingtimeChange = true;
    }

    static bool isStringTransmitted() {
        return stringIsTransmitted;
    }

 protected:
    static uint8_t stringPaddingTime;
    static uint8_t nextPaddingTime;
    static bool paddingtimeChange;
    static bool stringIsTransmitted;

 private:
    static const uint16_t bufferLength = 64;
    static uint32_t stringLengthBuffer[bufferLength];
    static uint16_t stringLengthIndexIn;
    static uint16_t stringLengthIndexOut;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_TIMEDSERIALDRIVER_H_
