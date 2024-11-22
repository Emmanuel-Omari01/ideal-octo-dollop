/*
 * MicrocontrollerConfig.cpp
 *
 *  Created on: 06.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "MicrocontrollerConfig.h"
// #include "../../../Renesas/Generated/r_bsp/mcu/rx111/register_access/gnuc/iodefine.h"

#include <platform.h>
// #include "../WatchDog/"

namespace AME_SRC {

MicrocontrollerConfig::MicrocontrollerConfig() = default;

MicrocontrollerConfig::~MicrocontrollerConfig() = default;

void MicrocontrollerConfig::disableWriteProtection() {
    // Enable writing to registers related to operating modes, LPC, CGC and software reset
    SYSTEM.PRCR.WORD = 0xA507U;

    // Enable writing to MPC pin function control registers
    MPC.PWPR.BIT.B0WI = 0U;
    MPC.PWPR.BIT.PFSWE = 1U;
}

void MicrocontrollerConfig::enableWriteProtection() {
    // Disable writing to MPC pin function control registers
    MPC.PWPR.BIT.PFSWE = 0U;
    MPC.PWPR.BIT.B0WI = 1U;

    // Enable protection
    SYSTEM.PRCR.WORD = 0xA500U;
}

void MicrocontrollerConfig::softwareReset() {
    SYSTEM.PRCR.WORD = 0xA507U;
    SYSTEM.SWRR = 0xA501;
}

void MicrocontrollerConfig::initCpuClockGenerator() {
    disableWriteProtection();

    // Set the System-Clock
    // DS: 30.08.2017 RX111 AME-Prototypen-Board 2017
    // Main-Quartz 16MHz, the cutoff frequency is 32 MHz
    // It is possible to set the STC factor in the PLLCR register to 8:
    // 16MHz *8/4 = 32 MHz. Actually 16MHz * 6/2 = 24 MHz !!!
    // page 156, PLLCR-Register *6/2 => 48MHz as CPU main clock
    // page 153, SCKCR-Register: PCKD, Peripheral Module Clock D - pre-scale 1/2
    //                           PCKB, Peripheral Module Clock B - pre-scale 1/2
    //                           ICK, System Clock (ICLK)        - pre-scale 1/2 ???
    //                           FlashIF (FLCK)                  - pre-scale 1/2

    // Set main clock control registers
    SYSTEM.MOFCR.BYTE = _00_CGC_MAINOSC_RESONATOR | _20_CGC_MAINOSC_OVER10M;  // b5=1 => 10...20MHz (16 MHz assembled)
    SYSTEM.MOSCWTCR.BYTE = _00_CGC_OSC_WAIT_CYCLE_2;

    // Set main clock operation
    SYSTEM.MOSCCR.BIT.MOSTP = 0U;       // 0=not stopped
//      /* Set IWDT */
//      SYSTEM.ILOCOCR.BIT.ILCSTP = 0U;

    // Wait for main clock oscillator wait counter overflow
    while (1U != SYSTEM.OSCOVFSR.BIT.MOOVF) { }

    // Set system clock, see hardware document page 153
    SYSTEM.SCKCR.LONG = _00000001_CGC_PCLKD_DIV_2 | _00000100_CGC_PCLKB_DIV_2
            | _01000000_CGC_ICLK_DIV_2 | _10000000_CGC_FCLK_DIV_2;

    // Set PLL circuit
    SYSTEM.PLLCR2.BIT.PLLEN = 0U;
    SYSTEM.PLLCR.WORD = _0001_CGC_PLL_FREQ_DIV_2 | _0B00_CGC_PLL_FREQ_MUL_6;  // 16MHz *6/2 => 48MHz
                                                         // until 29.11.2017: _0B00_CGC_PLL_FREQ_MUL_6
    // Wait for PLL wait counter overflow
    while (1U != SYSTEM.OSCOVFSR.BIT.PLOVF) {}

    // Disable sub-clock
    SYSTEM.SOSCCR.BIT.SOSTP = 1U;

    // Wait for the register modification to complete
    while (1U != SYSTEM.SOSCCR.BIT.SOSTP) {}

    // Set clock source: CKSEL[2:0] = 100b = PLL, hardware document page 155
    SYSTEM.SCKCR3.WORD = _0400_CGC_CLOCKSOURCE_PLL;

    enableWriteProtection();
}

void MicrocontrollerConfig::createCMT0() {
    /* Clock Select (CKS[1:0]) */
    const uint16_t _0003_CMT_CMCR_CKS_PCLK512 = (0x0003U);  // PCLK/512
    /* Compare Match Interrupt Enable (CMIE) */
    const uint16_t _0040_CMT_CMCR_CMIE_ENABLE = (0x0040U);  // Compare match interrupt (CMIn) enabled
    const uint16_t _0005_CMT0_CMCOR_VALUE = (0x0005U);      // Compare Match Timer Constant Register (CMCOR)
    /* Interrupt Priority Level Select (IPR[3:0]) */
    const uint16_t _0C_CMT_PRIORITY_LEVEL12 = (0x0CU);  /* Level 12 */
    disableWriteProtection();
    IEN(CMT0, CMI0) = 0U;               // Disable CMI interrupt
    SYSTEM.MSTPCRA.BIT.MSTPA15 = 0U;    // Cancel CMT stop state in LPC

    // Set control registers
    CMT0.CMCR.WORD = _0003_CMT_CMCR_CKS_PCLK512 | _0040_CMT_CMCR_CMIE_ENABLE;
    CMT0.CMCOR = _0005_CMT0_CMCOR_VALUE;

    // Set CMI0 priority level
    IPR(CMT0, CMI0) = _0C_CMT_PRIORITY_LEVEL12;
    enableWriteProtection();
}

void MicrocontrollerConfig::stopCMT0() {
    /* Stop CMT0 count */
    CMT.CMSTR0.BIT.STR0 = 0U;

    /* Disable CMI0 interrupt in ICU */
    IEN(CMT0, CMI0) = 0U;
}

void MicrocontrollerConfig::startCMT0() {
    /* Start CMT0 count */
   CMT.CMSTR0.BIT.STR0 = 1U;

   /* Enable CMI0 interrupt in ICU */
   IEN(CMT0, CMI0) = 1U;
}

}  // namespace AME_SRC

