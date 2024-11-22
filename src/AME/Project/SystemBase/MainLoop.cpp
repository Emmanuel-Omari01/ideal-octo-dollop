/*
 * MainLoop.cpp
 *
 *  Created on: 06.01.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "MainLoop.h"

#include "MicrocontrollerConfig.h"

#include "../../../Renesas/Generated/r_bsp/mcu/rx111/register_access/gnuc/iodefine.h"
#include "../../../Renesas/Generated/r_bsp/mcu/all/r_bsp_interrupts.h"

#ifdef CPPAPP
extern "C" {
#endif
#include "../../../Renesas/Generated/r_cmt_rx/r_cmt_rx_if.h"
#ifdef CPPAPP
}
#endif
extern "C" void osTimerIntRef(void * pdata);

namespace AME_SRC {

uint32_t MainLoop::microSecCounterLoop = 0;
uint16_t MainLoop::counterFlag = 0;
uint16_t MainLoop::microSecCounter100 = 0;
uint16_t MainLoop::milliSecCounter = 0;
uint16_t MainLoop::milliSecCounter5 = 0;
uint16_t MainLoop::milliSecCounter50 = 0;
uint8_t MainLoop::secCounter = 0;
bool MainLoop::hasStarted = false;

MainLoop::MainLoop() :
        execution100microPtr(0), execution1msPtr(0), execution5msPtr(0),
        execution50msPtr(0), execution1sPtr(0) {
    stop();
    initOSTimer();
}

MainLoop::~MainLoop() = default;

bool MainLoop::isStarted() {
    return hasStarted;
}

void MainLoop::start() {
  hasStarted = true;
}

void MainLoop::stop() {
  hasStarted = false;
}

void MainLoop::executePtrIfNotNull(void (*&executionPtr)()) {
    if (executionPtr) {
        executionPtr();
    }
}

void MainLoop::incrementCounters() {
    counterFlag = true;
    // microSecCounterLoop++;
}

void __attribute__((optimize("O0"))) MainLoop::executeUntilStop() {
    enum {
        time100micro = 1,
        time500micro = 5,
        time1ms = 10,
        time5ms = 5,
        time50ms = 10,
        time1s = 20,
        time60s = 60,
    };
    while (this->isStarted()) {
        if (counterFlag >= time100micro) {  // time100micro
            counterFlag = 0;
            microSecCounter100++;
            executePtrIfNotNull(execution100microPtr);

            if (microSecCounter100 >= time1ms) {
              microSecCounter100 = 0;
                milliSecCounter++;
                executePtrIfNotNull(execution1msPtr);
                if (milliSecCounter >= time5ms) {
                    milliSecCounter = 0;
                    milliSecCounter5++;
                    executePtrIfNotNull(execution5msPtr);
                    if (milliSecCounter5 >= time50ms) {
                        milliSecCounter5 = 0;
                        milliSecCounter50++;
                        executePtrIfNotNull(execution50msPtr);
                        if (milliSecCounter50 >= time1s) {
                            milliSecCounter50 = 0;
                            secCounter++;
                            executePtrIfNotNull(execution1sPtr);
                        }
                        if (secCounter >= time60s) {
                            executePtrIfNotNull(execution60sPtr);
                            secCounter = 0;
                        }
                    }
                }
            }
        }
    }
}

bool MainLoop::counterIsMultipleOfTime(uint32_t counter, uint32_t time) {
    return ((counter % time) == 0 && (counter != 0));
}

void MainLoop::initOSTimer() {
    /* Clock Select (CKS[1:0]) */
    // DS- const uint16_t _0001_CMT_CMCR_CKS_PCLK32 = (0x0001U); // PCLK/32
    /* Compare Match Interrupt Enable (CMIE) */
    const uint16_t _0040_CMT_CMCR_CMIE_ENABLE = (0x0040U);  // Compare match interrupt (CMIn) enabled
    /* Read/Write Enable (RWE) */
    /* Interrupt Priority Level Select (IPR[3:0]) */
    const uint8_t _0C_CMT_PRIORITY_LEVEL12 = (0x0CU); /* Level 12 */
    /* Interrupt Priority Level Select (IPR[3:0]) */
    const uint32_t channel0 = 0;

    // The following function is used to register CMT0
    R_CMT_CreatePeriodicAssignChannelPriority(2500, &osTimerIntRef, channel0, CMT_PRIORITY_12);

    IEN(CMT0, CMI0)= 0U;                // Disable CMI interrupt
    SYSTEM.MSTPCRA.BIT.MSTPA15 = 0U;    // Cancel CMT stop state in LPC

    // Set control registers
    // CMCR-Register hardware manual page 568
    // CMT0.CMCR.WORD = _0002_CMT_CMCR_CKS_PCLK128 | _0040_CMT_CMCR_CMIE_ENABLE;    // 24MHz / 128 = 187500Hz
    // CMT0.CMCOR = 1875;                   // => 10ms: 0.01s/(1/187500)
    CMT0.CMCR.WORD = (0x0000U) | _0040_CMT_CMCR_CMIE_ENABLE | (0x0080U);  // 24MHz / 32 = 750 kHz
    CMT0.CMCOR = (0x012BU);
    // from  26.07.2018  75: => 100µs: 0.0001s/(1/750000)
    // until 26.07.2018 750: =>   1ms: 0.001s/(1/750000)

    IPR(CMT0, CMI0)= _0C_CMT_PRIORITY_LEVEL12;  // Set CMI0 priority level
    IEN(CMT0, CMI0)= 1U;                        // Enable CMI0 interrupt in ICU
    CMT.CMSTR0.BIT.STR0 = 1U;                   // Start CMT0 count
    // R_BSP_InterruptWrite(BSP_INT_CMD_FIT_INTERRUPT_ENABLE, (bsp_int_cb_t) osTimerInterrupt);  // INT_CMT0_CMIO

    /*** DS, AME unused MTU timer function
    const uint8_t _01_MTU_RWE_ENABLE = (0x01U);  // Read/write access to the registers is enabled
    const uint8_t _08_MTU_PRIORITY_LEVEL8 = (0x08U);  // Level 8
    // Time Prescaler Select (TPSC[2:0])
    const uint8_t _03_MTU_PCLK_64 = (0x03U);  // Internal clock: counts on PCLK/64
    // Clock Edge Select (CKEG[1:0])
    const uint8_t _00_MTU_CKEG_RISE = (0x00U);  // Count at rising edge
    // Mode Select (MD[3:0])
    const uint8_t _00_MTU_NORMAL = (0x00U);  // Normal mode
    // Read/Write Enable (RWE)
    const uint8_t _00_MTU_RWE_DISABLE = (0x00U);  // Read/write access to the registers is disabled

    // Initialize Timer MTU0 to generate a rate of 1µs
    MTU.TRWER.BYTE = _01_MTU_RWE_ENABLE;  // Timer Read/Write Enable Registers:
                                          //  1 = read/write function for timers MTU3, MTU4
                                          // => protection disable = writing enabled
    // Stop channel MTU0
    MTU.TSTR.BIT.CST0 = 0;

    // Set interrupt priority level: MTU0.TCNT overflow
    IPR(MTU0, TCIV0)= _08_MTU_PRIORITY_LEVEL8;
    IEN(MTU0, TCIV0)= 1U;                  // Enable interrupt in ICU

    // Channel 0 to measure times. free running timer
    // DS: PCLK = 24 MHz, prescaler 64 => 375 kHz. 2,66666µs / Clock
    MTU0.TCR.BYTE = _03_MTU_PCLK_64 | _00_MTU_CKEG_RISE;  // prescaler: 1/64 PCLK, select flank, no clearing
    MTU0.TGRA = 0xFFFF;

//  MTU0.TGRB = _000A_TGRB_VALUE;          // Compare-Match Timer B comparing value
    MTU0.TIER.BIT.TCIEV = 1;    // Timer Interrupt Enable Register: look at hardware_manual page 392
//  MTU0.TIORH.BYTE |= _50_MTU_IOB_HL | _02_MTU_IOA_LH;  // Timer I/O Control Register H:
            // 0x50: Timer B Initial output is high, Low output at compare match.
            // 0x02: Timer A Initial output is low.  High output at compare match.
//  MTU0.TIORL.BYTE |= _00_MTU_IOC_DISABLE;  // Timer I/O Control Register L: MTIOC3C Pin Function: no output???
//  MTU.TSYR.BYTE |= _00_MTU_SYNC3_OFF;      // Timer Synchronous Register
                                             // => all Timer MTU0..MTU4 work independent of each other
    MTU0.TMDR.BYTE = _00_MTU_NORMAL;       // Timer Mode Register 0x00 = "normal" = no dependencies on other timers
    MTU.TRWER.BYTE = _00_MTU_RWE_DISABLE;  // Timer Read/Write Enable Registers:
                                           // 0=no write/read Function for timer MTU3, MTU4
                                           // => protect function for undesired writing access

    MTU.TSTR.BIT.CST0 = 1;      // Start MTU0
    ***/
}

}  // namespace AME_SRC

void osTimerInterrupt() {
    AME_SRC::MainLoop::counterFlag++;
}

void osTimerIntRef(void * pdata) {
    AME_SRC::MainLoop::counterFlag++;
}
