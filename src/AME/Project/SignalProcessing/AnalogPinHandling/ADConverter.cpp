/*
 * ADConverter.cpp
 *
 *  Created on: 17.11.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "ADConverter.h"
#include "../../SystemBase/MicrocontrollerConfig.h"

namespace AME_SRC {

bool ADConverter::isNotInstanciated = true;
bool ADConverter::conversionFinnished = true;
ADConverter* ADConverter::singlePtr = 0;

enum {start = 1, enabled = 1, asynchron = 1, highSpeed = 1, stop = 0, disabled = 0, synchron = 0, normalSpeed = 0};
#define scanRegisterModeSettings S12AD.ADCSR.BIT.ADCS
#define scanInterruptGroupeState S12AD.ADCSR.BIT.ADIE
#define scanInterruptFlag IR(S12AD, S12ADI0)
#define scanDoubleTriggerState S12AD.ADCSR.BIT.DBLE
#define scanInterruptGroupBState S12AD.ADCSR.BIT.GBADIE
#define scanTriggerSelection S12AD.ADCSR.BIT.EXTRG
#define scanTriggerStartState S12AD.ADCSR.BIT.TRGE
#define scanConversionModeSettings   S12AD.ADCSR.BIT.ADHSC
#define scanConversionStartState S12AD.ADCSR.BIT.ADST
ADConverter::ADConverter() {
    MicrocontrollerConfig::disableWriteProtection();
    isNotInstanciated = false;
    SYSTEM.MSTPCRA.BIT.MSTPA17 = disabled;  // Renesas: Cancel S12AD module stop state
    scanInterruptGroupeState = disabled;
    scanInterruptFlag = 0;
    IEN(S12AD, S12ADI0)= 0U;
    scanRegisterModeSettings = SingleScan;
    scanDoubleTriggerState = disabled;
    scanInterruptGroupBState = disabled;
    scanTriggerSelection = synchron;
    scanTriggerStartState = disabled;
    scanConversionModeSettings = normalSpeed;
    IPR(S12AD, S12ADI0) = 15;
    MPC.PJ6PFS.BIT.ASEL = 0;
    MPC.PJ7PFS.BIT.ASEL = 0;
    MicrocontrollerConfig::enableWriteProtection();
}

ADConverter* ADConverter::getInstance() {
    if (isNotInstanciated) {
      singlePtr = new ADConverter();
    }
    return singlePtr;
}

void ADConverter::startConversion() {
    scanInterruptFlag = 0;
    IEN(S12AD, S12ADI0) = 1U;
    scanConversionStartState = start;
    conversionFinnished = false;
}

void ADConverter::stopConversion() {
    scanConversionStartState = stop;
    IEN(S12AD, S12ADI0)= 0U;
    scanInterruptFlag = 0;
}

void ADConverter::enableConversionInGroupAForChannel(scanChannel channel) {
    S12AD.ADANSA.WORD |= 1 << channel;
}

// Read analog value from channel
// TODO(AME): convert register value to double value
double ADConverter::getDataFromChannel(scanChannel channel) {
    uint16_t result = 0;
    switch (channel) {
    case channel0:
        result = S12AD.ADDR0;
        break;
    case channel1:
        result = S12AD.ADDR1;
        break;
    case channel2:
        result = S12AD.ADDR2;
        break;
    case channel3:
        result = S12AD.ADDR3;
        break;
    case channel4:
        result = S12AD.ADDR4;
        break;
    case channel6:
        result = S12AD.ADDR6;
        break;
    case channel8:
        result = S12AD.ADDR8;
        break;
    case channel9:
        result = S12AD.ADDR9;
        break;
    case channel10:
        result = S12AD.ADDR10;
        break;
    case channel11:
        result = S12AD.ADDR11;
        break;
    case channel12:
        result = S12AD.ADDR12;
        break;
    case channel13:
        result = S12AD.ADDR13;
        break;
    case channel14:
        result = S12AD.ADDR14;
        break;
    case channel15:
        result = S12AD.ADDR15;
        break;
    default:
        break;
    }
    return result;
}

void ADConverter::waitForConversionCompleted() {
    while (scanConversionStartState) {  // = S12AD.ADCSR.BIT.ADST
        asm("NOP");                 // conversion time: ~2µs
    }
}

ADConverter::~ADConverter() {
    conversionFinnished = true;
}

}  // namespace AME_SRC

void s12AD0_callback() {
    AME_SRC::ADConverter::getInstance()->setConversionFinnished(true);
}
