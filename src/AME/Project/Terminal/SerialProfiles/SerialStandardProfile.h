/*
 * SerialStandardProfile.h
 *
 *  Created on: 06.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALSTANDARDPROFILE_H_
#define SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALSTANDARDPROFILE_H_

#include "SerialProfile.h"

namespace AME_SRC {

class SerialStandardProfile: public SerialProfile {
 private:
//  static const uint8_t cmdCount = 4;
//  static const uint16_t cmdLength = 256;
//  struct {
//      char container[cmdCount][cmdLength];
//      uint16_t indexIN;
//      uint16_t indexOUT;
//  } cmdLines;
    void fillNextCommandWithString(char *targetStringPtr);

 public:
    enum {
        RETURN = 0xd,
        LINEFEED = 0xa,
        ESC = 0x1b,
        DEL = 0x7f,
        BACKSPACE = 0x8,
        STX = 0x02,
        ETX = 0x03,
        SOH = 0x01,  // start of heading
        EOT = 0x04,  // end of transmission.
        ACK = 0x06,  // Acknowledge
        NAK = 0x15,  // neg. acknowledge
        _SI = 0x0F,  // shift in
        _SO = 0x0E,  // shift out
        ETB = 0x17,
        US = 0x1F,  // unit separator
    };
    SerialStandardProfile();
    virtual ~SerialStandardProfile();
    void interpretReceivedLetter(char letter);
    char* getBuffer();
    void setBuffer(uint8_t count, char *array);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALSTANDARDPROFILE_H_
