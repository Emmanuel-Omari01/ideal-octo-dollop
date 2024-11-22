/*
 * CompareMatchTimeDriver.cpp
 *
 *  Created on: 02.06.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "CompareMatchTimeDriver.h"

namespace AME_SRC {

const uint16_t  CompareMatchTimeDriver::_0000_CMT_CMCR_CLOCK_PCLK8    =(0x0000U); /* PCLK/8 */
const uint16_t  CompareMatchTimeDriver::_0001_CMT_CMCR_CLOCK_PCLK32   =(0x0001U); /* PCLK/32 */
const uint16_t  CompareMatchTimeDriver::_0002_CMT_CMCR_CLOCK_PCLK128  =(0x0002U); /* PCLK/128 */
const uint16_t  CompareMatchTimeDriver::_0003_CMT_CMCR_CLOCK_PCLK512  =(0x0003U); /* PCLK/512 */

CompareMatchTimeDriver::CompareMatchTimeDriver(uint16_t MicroDelay) {
  calculateClockAndMatchRegisterForMicroDelay(MicroDelay);
    // compareClock = _0003_CMT_CMCR_CLOCK_PCLK512;
    // matchRegisterValue = 5 ; // => 128µs @ PCLK512
}

CompareMatchTimeDriver::CompareMatchTimeDriver(uint16_t MicroDelay,
        void (*funcName)()) {
  calculateClockAndMatchRegisterForMicroDelay(MicroDelay);
}

void CompareMatchTimeDriver::setMatchInterruptFuncPtr(void (*funcName)()) {
}

CompareMatchTimeDriver::~CompareMatchTimeDriver() = default;

void CompareMatchTimeDriver::calculateClockAndMatchRegisterForMicroDelay(
        uint16_t delay) {
    if (delay > 0) {
        if (delay % 64 == 0) {
            setCompareClockAndMatchRegister(_0003_CMT_CMCR_CLOCK_PCLK512, delay);
        } else if (delay % 16 == 0) {
            setCompareClockAndMatchRegister(_0002_CMT_CMCR_CLOCK_PCLK128, delay);
        } else if (delay % 4 == 0) {
            setCompareClockAndMatchRegister(_0001_CMT_CMCR_CLOCK_PCLK32, delay);
        } else {
            setCompareClockAndMatchRegister(_0000_CMT_CMCR_CLOCK_PCLK8, delay);
        }
    }
}

void CompareMatchTimeDriver::setCompareClockAndMatchRegister(
        const uint16_t clock, uint16_t delay) {
    static const uint8_t delayDivisors[] = { 1, 4, 32, 64 };
    compareClock = clock;
    matchRegisterValue = 2 + ((delay / (delayDivisors[clock])) - 1) * 3;
}

}  // namespace AME_SRC

