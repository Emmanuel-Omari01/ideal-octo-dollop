/*
 * SerialCpxBootProfile.h
 *
 *  Created on: 20.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALCPXBOOTPROFILE_H_
#define SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALCPXBOOTPROFILE_H_

#include "SerialProfile.h"

#include <cstdint>

#include "../../PlcCommunication/ModemInterpreter.h"

namespace AME_SRC {

class SerialCpxBootProfile: public SerialProfile {
 private:
    ModemInterpreter *modemPtr_;

 public:
    explicit SerialCpxBootProfile(ModemInterpreter *modemPtr);
    virtual ~SerialCpxBootProfile();
    void interpretReceivedLetter(char letter);
    char* getBuffer();
    void setBuffer(uint8_t count, char *array);
    void setModemPtr(ModemInterpreter *&modemPtr) {
        modemPtr_ = modemPtr;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALCPXBOOTPROFILE_H_
