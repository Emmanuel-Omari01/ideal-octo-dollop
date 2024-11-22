/*
 * SerialProfile.h
 *
 *  Created on: 06.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALPROFILE_H_
#define SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALPROFILE_H_

#include "../../SystemBase/CustomTypeDefs.h"

namespace AME_SRC {

class SerialProfile {
 public:
    enum { cmdLength = 256 };

 protected:
    SerialProfile();
    static const uint8_t cmdCount = 4;
    struct {
        char container[cmdCount][cmdLength];
        uint16_t indexIN;
        uint16_t indexOUT;
    } cmdLines;

 public:
    virtual void interpretReceivedLetter(char letter) = 0;
    virtual char* getBuffer() = 0;
    virtual void setBuffer(uint8_t count, char *array) = 0;
    virtual ~SerialProfile() = 0;
};

}  // namespace AME_SRC

inline AME_SRC::SerialProfile::SerialProfile() { }
inline AME_SRC::SerialProfile::~SerialProfile() { }

#endif  // SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALPROFILE_H_
