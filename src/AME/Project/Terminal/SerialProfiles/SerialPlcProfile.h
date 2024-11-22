/*
 * SerialPlcProfile.h
 *
 *  Created on: 06.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALPLCPROFILE_H_
#define SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALPLCPROFILE_H_

#include "SerialProfile.h"

#include <cstdint>

#include "../../PlcCommunication/ModemInterpreter.h"
#include "../../PlcCommunication/AliveHandling/MessageForwarding/ForwardingFlow.h"

namespace AME_SRC {

class SerialPlcProfile: public SerialProfile {
 private:
    uint8_t frameCount;

    void fillNextCommandWithString(char *targetStringPtr);
    // struct CheckAlivePatternResult {
    //  enum type { SendToInterpreter=1, SendInterrupt, SendIntAndInterpreter }; // {1,2,3}
    //};
    int searchForPattern(char *pattern);
    bool isBroadcastFrame(char *CurCmd, int16_t idx);
    uint16_t getDestGroupIp(char *CurCmd);
    ModemInterpreter *modemPtr;
    ForwardingFlow &aliveForwarding_;

 public:
    SerialPlcProfile(ForwardingFlow &forwardFlow);
    virtual ~SerialPlcProfile();
    void interpretReceivedLetter(char letter);
    char* getBuffer();
    void setBuffer(uint8_t count, char *array);
    bool alive_rx_matched;

    void setModemPtr(ModemInterpreter *modemPtr) {
        this->modemPtr = modemPtr;
    }

    uint8_t getFramingCount() const {
        return frameCount;
    }

    void setFramingCount(uint8_t frameCount) {
        this->frameCount = frameCount;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_SERIALPROFILES_SERIALPLCPROFILE_H_
