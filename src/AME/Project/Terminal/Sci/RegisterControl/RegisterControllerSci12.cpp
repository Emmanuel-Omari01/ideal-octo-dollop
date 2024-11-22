/*
 * RegisterControllerSci12.cpp
 *
 *  Created on: 21.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "RegisterControllerSci12.h"

#include <platform.h>

namespace AME_SRC {

RegisterControllerSci12::RegisterControllerSci12() = default;

RegisterControllerSci12::~RegisterControllerSci12() = default;

void RegisterControllerSci12::setRegisterByteSMRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.SMR.BYTE = byte;
}

void RegisterControllerSci12::setRegisterByteSCRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.SCR.BYTE = byte;
}

void RegisterControllerSci12::setRegisterByteBRRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.BRR = byte;
}

void RegisterControllerSci12::setRegisterByteSEMRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.SEMR.BYTE = byte;
}

void RegisterControllerSci12::setRegisterByteSPMRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.SPMR.BYTE = byte;
}

void RegisterControllerSci12::setRegisterByteSCMRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.SCMR.BYTE = byte;
}

void RegisterControllerSci12::setRegisterByteTDRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.TDR = byte;
}

void RegisterControllerSci12::setRegisterByteSSRforSCI(uint8_t byte, SCINumber sci) {
    SCI12.SSR.BYTE = byte;
}

uint8_t RegisterControllerSci12::getRegisterByteSCRforSCI(SCINumber sci) {
    return SCI12.SCR.BYTE;
}

uint8_t RegisterControllerSci12::getRegisterByteSSRforSCI(SCINumber sci) {
    return SCI12.SSR.BYTE;
}

uint8_t RegisterControllerSci12::getRegisterByteRDRforSCI(SCINumber sci) {
    return SCI12.RDR;
}

}  // namespace AME_SRC
