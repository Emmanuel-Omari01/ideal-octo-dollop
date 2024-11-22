/*
 * MicrocontrollerConfig.h
 *
 *  Created on: 06.01.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_MICROCONTROLLERCONFIG_H_
#define SRC_AME_PROJECT_SYSTEMBASE_MICROCONTROLLERCONFIG_H_

#include <stdint.h>

namespace AME_SRC {

class MicrocontrollerConfig {
 private:
    /* Main Clock Oscillator Switch (MOSEL) */
    static const uint8_t _00_CGC_MAINOSC_RESONATOR = (0x00U);  // Resonator
    /* Main Oscillator Drive Capability Switch (MODRV21) */
    static const uint8_t _20_CGC_MAINOSC_OVER10M = (0x20U);  // 10 MHz to 20 MHz
    /* Main Clock Oscillator Wait Time (MSTS[4:0]) */
    static const uint8_t _00_CGC_OSC_WAIT_CYCLE_2 = (0x00U);  // Wait time = 2 cycles

    // System Clock Control Register (SCKCR)
    /* Peripheral Module Clock D (PCLKD) */
    static const uint32_t _00000001_CGC_PCLKD_DIV_2 = (0x00000001UL); /* x1/2 */
    /* Peripheral Module Clock B (PCLKB) */
    static const uint32_t _00000100_CGC_PCLKB_DIV_2 = (0x00000100UL); /* x1/2 */
    /* System Clock (ICLK) */
    static const uint32_t _01000000_CGC_ICLK_DIV_2 = (0x01000000UL); /* x1/2 */
    /* System Clock (FCLK) */
    static const uint32_t _10000000_CGC_FCLK_DIV_2 = (0x10000000UL); /* x1/2 */

    // PLL Control Register (PLLCR)
    /* PLL Input Frequency Division Ratio Select (PLIDIV[1:0]) */
    static const uint16_t _0001_CGC_PLL_FREQ_DIV_2 = (0x0001U); /* x1/2 */
    /* Frequency Multiplication Factor Select (STC[5:0]) */
    static const uint16_t _0B00_CGC_PLL_FREQ_MUL_6 = (0x0B00U); /* x6 */
    //    System Clock Control Register 3 (SCKCR3)
    static const uint16_t _0400_CGC_CLOCKSOURCE_PLL = (0x0400U); /* PLL circuit */

 public:
    static void enableWriteProtection();
    static void disableWriteProtection();
    static void softwareReset();
    MicrocontrollerConfig();
    static void initCpuClockGenerator();
    static void createCMT0();
    static void stopCMT0();
    static void startCMT0();

    virtual ~MicrocontrollerConfig();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_MICROCONTROLLERCONFIG_H_

