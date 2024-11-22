/*
 * SciRegisterController.cpp
 *
 *  Created on: 05.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "SciRegisterController.h"

#include <platform.h>

namespace AME_SRC {

SciRegisterController::SciRegisterController() = default;

SciRegisterController::~SciRegisterController() = default;

void SciRegisterController::setRegisterByteSMRforSCI(uint8_t byte, SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.SMR.BYTE = byte;
        break;
    case sci5:
        SCI5.SMR.BYTE = byte;
        break;
    case sci12:
        SCI12.SMR.BYTE = byte;
        break;
    }
}

void SciRegisterController::setRegisterByteSCRforSCI(uint8_t byte, SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.SCR.BYTE = byte;
        break;
    case sci5:
        SCI5.SCR.BYTE = byte;
        break;
    case sci12:
        SCI12.SCR.BYTE = byte;
        break;
    }
}

void SciRegisterController::setRegisterByteBRRforSCI(uint8_t byte, SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.BRR = byte;
        break;
    case sci5:
        SCI5.BRR = byte;
        break;
    case sci12:
        SCI12.BRR = byte;
        break;
    }
}

void SciRegisterController::setRegisterByteSEMRforSCI(uint8_t byte, SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.SEMR.BYTE = byte;
        break;
    case sci5:
        SCI5.SEMR.BYTE = byte;
        break;
    case sci12:
        SCI12.SEMR.BYTE = byte;
        break;
    }
}

void SciRegisterController::setRegisterByteSPMRforSCI(uint8_t byte, SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.SPMR.BYTE = byte;
        break;
    case sci5:
        SCI5.SPMR.BYTE = byte;
        break;
    case sci12:
        SCI12.SPMR.BYTE = byte;
        break;
    }
}

void SciRegisterController::setRegisterByteSCMRforSCI(uint8_t byte, SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.SCMR.BYTE = byte;
        break;
    case sci5:
        SCI5.SCMR.BYTE = byte;
        break;
    case sci12:
        SCI12.SCMR.BYTE = byte;
        break;
    }
}

void SciRegisterController::setRegisterByteTDRforSCI(uint8_t byte, SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.TDR = byte;
        break;
    case sci5:
        SCI5.TDR = byte;
        break;
    case sci12:
        SCI12.TDR = byte;
        break;
    }
}

void SciRegisterController::setRegisterByteTDRforSCI1(uint8_t byte) {
    SCI1.TDR = byte;
}

void SciRegisterController::setRegisterByteTDRforSCI5(uint8_t byte) {
    SCI5.TDR = byte;
}

void SciRegisterController::setRegisterByteTDRforSCI12(uint8_t byte) {
    SCI12.TDR = byte;
}

void SciRegisterController::setRegisterByteSSRforSCI(uint8_t byte,
        SCINumber sci) {
    switch (sci) {
    case sci1:
        SCI1.SSR.BYTE = byte;
        break;
    case sci5:
        SCI5.SSR.BYTE = byte;
        break;
    case sci12:
        SCI12.SSR.BYTE = byte;
        break;
    }
}

uint8_t SciRegisterController::getRegisterByteSCRforSCI(SCINumber sci) {
    uint8_t result;
    switch (sci) {
    case sci1:
        result = SCI1.SCR.BYTE;
        break;
    case sci5:
        result = SCI5.SCR.BYTE;
        break;
    case sci12:
        result = SCI12.SCR.BYTE;
        break;
    }
    return result;
}

uint8_t SciRegisterController::getRegisterByteSSRforSCI(SCINumber sci) {
    uint8_t result;
    switch (sci) {
    case sci1:
        result = SCI1.SSR.BYTE;
        break;
    case sci5:
        result = SCI5.SSR.BYTE;
        break;
    case sci12:
        result = SCI12.SSR.BYTE;
        break;
    }
    return result;
}

uint8_t SciRegisterController::getRegisterByteRDRforSCI(SCINumber sci) {
    uint8_t result;
    switch (sci) {
    case sci1:
        result = SCI1.RDR;
        break;
    case sci5:
        result = SCI5.RDR;
        break;
    case sci12:
        result = SCI12.RDR;
        break;
    }
    return result;
}

uint8_t SciRegisterController::getRegisterByteSSRforSCI1() {
    return SCI1.SSR.BYTE;
}

uint8_t SciRegisterController::getRegisterByteSSRforSCI5() {
    return SCI5.SSR.BYTE;
}

uint8_t SciRegisterController::getRegisterByteSSRforSCI12() {
    return SCI12.SSR.BYTE;
}

uint8_t SciRegisterController::getRegisterByteRDRforSCI1() {
    return SCI1.RDR;
}

uint8_t SciRegisterController::getRegisterByteRDRforSCI5() {
    return SCI5.RDR;
}

uint8_t SciRegisterController::getRegisterByteRDRforSCI12() {
    return SCI12.RDR;
}

}  // namespace AME_SRC

