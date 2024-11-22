/*
 * ModemTransmitter.cpp
 *
 *  Created on: 13.09.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "ModemTransmitter.h"

#include <cstring>

namespace AME_SRC {

SerialDriver* ModemTransmitter::modemInterface = NULL;
TermContextFilter* ModemTransmitter::termFilterPtr_ = NULL;

bool ModemTransmitter::notWaitingForConfirmation = true;
ModemTransmitter::ModemTransmitter(SerialDriver* driver, TermContextFilter* crier) {
    termFilterPtr_ = crier;
    modemInterface = driver;
}

ModemTransmitter::~ModemTransmitter() = default;

// void ModemTransmitter::printCommandInVerboseMode(const PlcCommand *requestPtr) {
//  if (termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCData) && requestPtr){
//      char* requestString = requestPtr->toCommandStringHead();
//      termFilterPtr_->println(requestString);
//      requestString = requestPtr->toCommandStringData();
//      termFilterPtr_->println(requestString);
//      termFilterPtr_->println("");
//      //delete[] requestString;
//  }
//}

#pragma GCC diagnostic ignored "-Wstack-usage="
void ModemTransmitter::announceJoinWithNetMemberAndAwarenessState(
    NetworkMember *netMember, bool awarenessState) {
    char dump[128];
    if (true == awarenessState) {
        termFilterPtr_->println(TX::cTxtNewJoinText);
    } else {
        termFilterPtr_->println(TX::cTxtReJoinText);
    }
    termFilterPtr_->println(netMember->toString(dump, sizeof(dump)));
}

/**
 * sends an PLC command with confirmation
 * Description: the method send a plc command only if the last command was acknowledged by the modem
 * @param request
 */
void ModemTransmitter::sendRequestWithConfirmation(PlcCommand *request) {
    if (notWaitingForConfirmation) {
        sendRequest(request);
    }
}

/**
 * sends an PLC command without confirmation
 * Description: In this method, the modem's UART driver uses a
 * PLC command sent. There is no waiting for confirmation
 * @param request
 */
void ModemTransmitter::sendRequest(PlcCommand *&request) {
    if (request) {
        char *msg = request->toRawString2();
        modemInterface->printRawFast(msg, request->getStringLength());
        notWaitingForConfirmation = false;
    }
}

}  // namespace AME_SRC

