/*
 * CompareTimer1.cpp
 *
 *  Created on: 09.11.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "CompareTimer1.h"

#include <platform.h>
#include "../MicrocontrollerConfig.h"

namespace AME_SRC {

void (*CompareTimer1::matchInterruptPtrChannel1)() = 0;

CompareTimer1::CompareTimer1(uint16_t delay) : CompareMatchTimeDriver(delay) {
    create();
}

CompareTimer1::CompareTimer1(uint16_t MikroDelay, void (*funcName)()) :
        CompareMatchTimeDriver(MikroDelay) {
    setMatchInterruptFuncPtr(funcName);
    create();
}

void CompareTimer1::create() {
    const uint16_t _0040_CMT_CMCR_CMIE_ENABLE = (0x0040U);
    const uint16_t _0080_CMT_CMCR_DEFAULT = (0x0080U); /* Write default value of CMCR */
    // const uint16_t  _04DC_CMT1_CMCOR_VALUE   =(0x04DCU);
    MicrocontrollerConfig::disableWriteProtection();
    IEN(CMT1, CMI1)= 0U;  /* Disable CMI1 interrupt */
    MSTP(CMT1) = 0U; /* Cancel CMT stop state in LPC */
    /* Set control registers */
    CMT1.CMCR.WORD = compareClock | _0040_CMT_CMCR_CMIE_ENABLE
            | _0080_CMT_CMCR_DEFAULT;
    /* Set compare match register */
//  CMT1.CMCOR = matchRegisterValue;    // see start()
    /* Set CMI1 priority level */
    IPR(CMT1, CMI1)= _08_CMT_PRIORITY_LEVEL8;
    MicrocontrollerConfig::enableWriteProtection();
}

void CompareTimer1::start() {
    CMT1.CMCOR = matchRegisterValue;

    /* Enable CMI1 interrupt in ICU */
    IEN(CMT1, CMI1)= 1U;

    /* Start CMT1 count */
    CMT.CMSTR0.BIT.STR1 = 1U;
}

void CompareTimer1::stop() {
    /* Stop CMT1 count */
    CMT.CMSTR0.BIT.STR1 = 0U;

    /* Disable CMI1 interrupt in ICU */
    IEN(CMT1, CMI1)= 0U;
}

CompareTimer1::~CompareTimer1() = default;

void CompareTimer1::setMatchInterruptFuncPtr(void (*funcName)()) {
    matchInterruptPtrChannel1 = funcName;
}

}  // namespace AME_SRC

void timerInterruptChannel1() {
    AME_SRC::CompareTimer1::matchInterruptPtrChannel1();
}
