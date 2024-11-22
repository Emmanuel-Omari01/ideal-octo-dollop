/*
 * RegisterControllerSci5.cpp
 *
 *  Created on: 21.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "RegisterControllerSci5.h"

#include <platform.h>

namespace AME_SRC {

RegisterControllerSci5::RegisterControllerSci5() = default;

RegisterControllerSci5::~RegisterControllerSci5() = default;

void RegisterControllerSci5::setRegisterByteSMRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.SMR.BYTE = byte;
}

void RegisterControllerSci5::setRegisterByteSCRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.SCR.BYTE = byte;
}

void RegisterControllerSci5::setRegisterByteBRRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.BRR = byte;
}

void RegisterControllerSci5::setRegisterByteSEMRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.SEMR.BYTE = byte;
}

void RegisterControllerSci5::setRegisterByteSPMRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.SPMR.BYTE = byte;
}

void RegisterControllerSci5::setRegisterByteSCMRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.SCMR.BYTE = byte;
}

void RegisterControllerSci5::setRegisterByteTDRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.TDR = byte;
}

void RegisterControllerSci5::setRegisterByteSSRforSCI(uint8_t byte, SCINumber sci) {
    SCI5.SSR.BYTE = byte;
}

uint8_t RegisterControllerSci5::getRegisterByteSCRforSCI(SCINumber sci) {
    return SCI5.SCR.BYTE;
}

uint8_t RegisterControllerSci5::getRegisterByteSSRforSCI(SCINumber sci) {
    return SCI5.SSR.BYTE;
}

uint8_t RegisterControllerSci5::getRegisterByteRDRforSCI(SCINumber sci) {
    return SCI5.RDR;
}

}  // namespace AME_SRC
