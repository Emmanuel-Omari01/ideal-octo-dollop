/*
 * ModemTransmitter.h
 *
 *  Created on: 13.09.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMTRANSMITTER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMTRANSMITTER_H_

#include <cstdint>

#include "../Terminal/SerialDrivers/SerialDriver.h"
#include "../Terminal/TermContextFilter.h"
#include "G3PlcSystem/PlcCommand.h"
#include "NetworkMember.h"

namespace AME_SRC {

class ModemTransmitter {
 private:
    static const char *const newJoinText;
    static const char *const reJoinText;

    static SerialDriver *modemInterface;
    static TermContextFilter* termFilterPtr_;
    static bool notWaitingForConfirmation;

    void sendRequestWithConfirmation(PlcCommand *request);

 public:
    void announceJoinWithNetMemberAndAwarenessState(NetworkMember *netMember, bool awarenessState);
    static void sendRequest(PlcCommand *&request);
    ModemTransmitter(SerialDriver *driver1, TermContextFilter *crier);
    virtual ~ModemTransmitter();
    // void printCommandInVerboseMode(const PlcCommand *requestPtr);
    bool isNotWaitingForConfirmation() const {
        return notWaitingForConfirmation;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMTRANSMITTER_H_
