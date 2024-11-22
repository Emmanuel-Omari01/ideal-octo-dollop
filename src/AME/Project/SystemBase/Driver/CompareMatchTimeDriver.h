/*
 * CompareMatchTimeDriver.h
 *
 *  Created on: 02.06.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPAREMATCHTIMEDRIVER_H_
#define SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPAREMATCHTIMEDRIVER_H_

#include <cstdint>

namespace AME_SRC {

class CompareMatchTimeDriver {
 protected:
static const uint16_t _0000_CMT_CMCR_CLOCK_PCLK8;    /* PCLK/8 */
static const uint16_t _0001_CMT_CMCR_CLOCK_PCLK32;   /* PCLK/32 */
static const uint16_t _0002_CMT_CMCR_CLOCK_PCLK128;  /* PCLK/128 */
static const uint16_t _0003_CMT_CMCR_CLOCK_PCLK512;  /* PCLK/512 */

/* Interrupt Priority Level Select (IPR[3:0]) */
static const uint16_t _00_CMT_PRIORITY_LEVEL0    = (0x00U);/* Level 0 (interrupt disabled) */
static const uint16_t _01_CMT_PRIORITY_LEVEL1    = (0x01U);/* Level 1 */
static const uint16_t _02_CMT_PRIORITY_LEVEL2    = (0x02U);/* Level 2 */
static const uint16_t _03_CMT_PRIORITY_LEVEL3    = (0x03U);/* Level 3 */
static const uint16_t _04_CMT_PRIORITY_LEVEL4    = (0x04U);/* Level 4 */
static const uint16_t _05_CMT_PRIORITY_LEVEL5    = (0x05U);/* Level 5 */
static const uint16_t _06_CMT_PRIORITY_LEVEL6    = (0x06U);/* Level 6 */
static const uint16_t _07_CMT_PRIORITY_LEVEL7    = (0x07U);/* Level 7 */
static const uint16_t _08_CMT_PRIORITY_LEVEL8    = (0x08U);/* Level 8 */
static const uint16_t _09_CMT_PRIORITY_LEVEL9    = (0x09U);/* Level 9 */
static const uint16_t _0A_CMT_PRIORITY_LEVEL10   = (0x0AU);/* Level 10 */
static const uint16_t _0B_CMT_PRIORITY_LEVEL11   = (0x0BU);/* Level 11 */
static const uint16_t _0C_CMT_PRIORITY_LEVEL12   = (0x0CU);/* Level 12 */
static const uint16_t _0D_CMT_PRIORITY_LEVEL13   = (0x0DU);/* Level 13 */
static const uint16_t _0E_CMT_PRIORITY_LEVEL14   = (0x0EU);/* Level 14 */
static const uint16_t _0F_CMT_PRIORITY_LEVEL15   = (0x0FU);/* Level 15 (highest) */


CompareMatchTimeDriver(uint16_t MicroDelay);
CompareMatchTimeDriver(uint16_t MicroDelay, void (*funcName)());

 public:
    virtual ~CompareMatchTimeDriver();
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setMatchInterruptFuncPtr(void (*funcName)()) = 0;
    void calculateClockAndMatchRegisterForMicroDelay(uint16_t delay);

 protected:
    uint16_t matchRegisterValue;
    uint16_t compareClock;
    // void (*matchInterruptPtr)();

 private:
    void setCompareClockAndMatchRegister(const uint16_t clock, uint16_t delay);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPAREMATCHTIMEDRIVER_H_
