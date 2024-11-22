/*
 * RegisterControllerSci1.cpp
 *
 *  Created on: 21.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "RegisterControllerSci1.h"

#include <platform.h>

namespace AME_SRC {

RegisterControllerSci1::RegisterControllerSci1() = default;

RegisterControllerSci1::~RegisterControllerSci1() = default;

void RegisterControllerSci1::setRegisterByteSMRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.SMR.BYTE = byte;
}

void RegisterControllerSci1::setRegisterByteSCRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.SCR.BYTE = byte;
}

void RegisterControllerSci1::setRegisterByteBRRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.BRR = byte;
}

void RegisterControllerSci1::setRegisterByteSEMRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.SEMR.BYTE = byte;
}

void RegisterControllerSci1::setRegisterByteSPMRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.SPMR.BYTE = byte;
}

void RegisterControllerSci1::setRegisterByteSCMRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.SCMR.BYTE = byte;
}

void RegisterControllerSci1::setRegisterByteTDRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.TDR = byte;
}

void RegisterControllerSci1::setRegisterByteSSRforSCI(uint8_t byte, SCINumber sci) {
    SCI1.SSR.BYTE = byte;
}

uint8_t RegisterControllerSci1::getRegisterByteSCRforSCI(SCINumber sci) {
    return SCI1.SCR.BYTE;
}

uint8_t RegisterControllerSci1::getRegisterByteSSRforSCI(SCINumber sci) {
    return SCI1.SSR.BYTE;
}

uint8_t RegisterControllerSci1::getRegisterByteRDRforSCI(SCINumber sci) {
    return SCI1.RDR;
}

}  // namespace AME_SRC
