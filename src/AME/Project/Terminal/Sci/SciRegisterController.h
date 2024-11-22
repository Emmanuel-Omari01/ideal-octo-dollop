/*
 * SciRegisterController.h
 *
 *  Created on: 05.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SCI_SCIREGISTERCONTROLLER_H_
#define SRC_AME_PROJECT_TERMINAL_SCI_SCIREGISTERCONTROLLER_H_

#include "SciParam.h"

#include "../../SystemBase/CustomTypeDefs.h"

namespace AME_SRC {

class SciRegisterController {
 public:
    SciRegisterController();
    virtual ~SciRegisterController();
    static void setRegisterByteSMRforSCI(uint8_t byte, SCINumber sci);
    static void setRegisterByteSCRforSCI(uint8_t byte, SCINumber sci);
    static void setRegisterByteBRRforSCI(uint8_t byte, SCINumber sci);
    static void setRegisterByteSEMRforSCI(uint8_t byte, SCINumber sci);
    static void setRegisterByteSPMRforSCI(uint8_t byte, SCINumber sci);
    static void setRegisterByteSCMRforSCI(uint8_t byte, SCINumber sci);

    static void setRegisterByteTDRforSCI(uint8_t byte, SCINumber sci);
    static void setRegisterByteTDRforSCI1(uint8_t byte);
    static void setRegisterByteTDRforSCI5(uint8_t byte);
    static void setRegisterByteTDRforSCI12(uint8_t byte);

    static void setRegisterByteSSRforSCI(uint8_t byte, SCINumber sci);

    static uint8_t getRegisterByteSCRforSCI(SCINumber sci);

    static uint8_t getRegisterByteSSRforSCI(SCINumber sci);
    static uint8_t getRegisterByteSSRforSCI1();
    static uint8_t getRegisterByteSSRforSCI5();
    static uint8_t getRegisterByteSSRforSCI12();

    static uint8_t getRegisterByteRDRforSCI(SCINumber sci);
    static uint8_t getRegisterByteRDRforSCI1();
    static uint8_t getRegisterByteRDRforSCI5();
    static uint8_t getRegisterByteRDRforSCI12();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_SCI_SCIREGISTERCONTROLLER_H_
