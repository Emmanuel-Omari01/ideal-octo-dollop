/*
 * CompareTimer0.cpp
 *
 *  Created on: 02.06.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "CompareTimer0.h"

#include <cstdint>

#include <platform.h>
#include "../MicrocontrollerConfig.h"
#include "CompareMatchTimeDriver.h"

namespace AME_SRC {

void (*CompareTimer0::matchInterruptPtrChannel0)() = 0;

CompareTimer0::CompareTimer0(uint16_t delay) : CompareMatchTimeDriver(delay) {
    create();
}

CompareTimer0::CompareTimer0(uint16_t MicroDelay, void (*funcName)()) :
        CompareMatchTimeDriver(MicroDelay) {
    setMatchInterruptFuncPtr(funcName);
    create();
}

void CompareTimer0::create() {
    /* Compare Match Interrupt Enable (CMIE) */
    const uint16_t _0040_CMT_CMCR_CMIE_ENABLE = (0x0040U);  // Compare match interrupt (CMIn) enabled
    // const uint16_t _0005_CMT0_CMCOR_VALUE = (0x0005U);   // Compare Match Timer Constant Register (CMCOR)
    /* Interrupt Priority Level Select (IPR[3:0]) */
    const uint16_t _0C_CMT_PRIORITY_LEVEL12 = (0x0CU); /* Level 12 */
    MicrocontrollerConfig::disableWriteProtection();
    IEN(CMT0, CMI0) = 0U;    // Disable CMI interrupt
    SYSTEM.MSTPCRA.BIT.MSTPA15 = 0U;    // Cancel CMT stop state in LPC

    // Set control registers
    CMT0.CMCR.WORD = compareClock | _0040_CMT_CMCR_CMIE_ENABLE;
//  CMT0.CMCOR = matchRegisterValue;    // see start()

    // Set CMI0 priority level
    IPR(CMT0, CMI0)= _0C_CMT_PRIORITY_LEVEL12;
    MicrocontrollerConfig::enableWriteProtection();
}

void CompareTimer0::start() {
    CMT0.CMCOR = matchRegisterValue;

    /* Enable CMI0 interrupt in ICU */
    IEN(CMT0, CMI0)= 1U;

    /* Start CMT0 count */
    CMT.CMSTR0.BIT.STR0 = 1U;
}

void CompareTimer0::stop() {
    /* Stop CMT0 count */
    CMT.CMSTR0.BIT.STR0 = 0U;

    /* Disable CMI0 interrupt in ICU */
    IEN(CMT0, CMI0)= 0U;
}

CompareTimer0::~CompareTimer0() = default;

void CompareTimer0::setMatchInterruptFuncPtr(void (*funcName)()) {
    matchInterruptPtrChannel0 = funcName;
}

}  // namespace AME_SRC

void timerInterruptChannel0() {
    AME_SRC::CompareTimer0::matchInterruptPtrChannel0();
}

