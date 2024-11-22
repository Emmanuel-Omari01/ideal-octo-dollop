/*
 * SerialPlcProfile.cpp
 *
 *  Created on: 06.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "SerialPlcProfile.h"
#include <string.h>
#include "../Sci/SerialComInterface.h"
#include "../../SystemBase/ProgramConfig.h"
#include "../../PlcCommunication/G3PlcSystem/PlcCommand.h"
#include "../../PlcCommunication/ModemController.h"
#include "../../PlcCommunication/ModemInterpreter.h"
#include "../../PlcCommunication/PlcTerminalCommandConnector.h"

namespace AME_SRC {

#define ALIVE_MIN_START_POS 50  // minimum length of a command for a receiving string, which contains the
                                // Alive_RX_Pattern of an alive message
const uint8_t Alive_RX_Pattern[4] = { ':', ':', 'A', 0x00 };  // template for an alive check at peer

SerialPlcProfile::SerialPlcProfile(ForwardingFlow &forwardFlow) :
    frameCount(0), alive_rx_matched(false), aliveForwarding_(forwardFlow) {
    cmdLines.indexIN = 0;
    cmdLines.indexOUT = 0;
    modemPtr = 0;
}

SerialPlcProfile::~SerialPlcProfile() = default;

void SerialPlcProfile::interpretReceivedLetter(char letter) {
    static char currentCmd[cmdLength];

    bool lCopy = 0;
    static int16_t rxd_index = 0;
    switch (letter) {
    case PlcCommand::framingCode:
        frameCount++;
        currentCmd[rxd_index++] = letter;
        if (frameCount >= 2) {
            if (rxd_index <= 7) {
              frameCount = 1;
                rxd_index = 1;
            } else if (rxd_index > 7
                    && currentCmd[0] == PlcCommand::framingCode) {
                lCopy = true;
              frameCount = 0;
            } else {
                rxd_index = 0;
              frameCount = 0;
            }
        }
        break;
    default:
        currentCmd[rxd_index++] = letter;
        break;
    }
    if (lCopy) {
        int16_t tmp_len = rxd_index;
        memset(&currentCmd[tmp_len], 0, cmdLength - tmp_len);  // terminate line and delete rest of line
        rxd_index = 0;
        if (isBroadcastFrame(currentCmd, tmp_len)) {
            aliveForwarding_.loadAndProofAliveCmd(currentCmd, tmp_len);
            if (aliveForwarding_.isAliveCmdValid()) {
              aliveForwarding_.setReceivedIp(getDestGroupIp(currentCmd));
                aliveForwarding_.processForwarding();
                if (ProgramConfig::getTerminalFilter()->
                        isNotFiltered(TermContextFilter::filterAliveRxInfo)) {
                    fillNextCommandWithString(currentCmd);  // DEBUG
                }
            } else {
                fillNextCommandWithString(currentCmd);
            }
        } else {
            fillNextCommandWithString(currentCmd);
        }
    }
}

void SerialPlcProfile::fillNextCommandWithString(char *targetStringPtr) {
    char *destinationPtr;
    destinationPtr = &cmdLines.container[cmdLines.indexIN][0];
    memcpy(destinationPtr, targetStringPtr, cmdLength);  // Copy characters for evaluation
    memset(targetStringPtr, 0, cmdLength);  // suppress doubled interpretation
    // trigger Interpreter
    if (++cmdLines.indexIN >= cmdCount) {
        cmdLines.indexIN = 0;
    }
}

char* SerialPlcProfile::getBuffer() {
    char *p;
    if (cmdLines.indexOUT != cmdLines.indexIN) {
        p = cmdLines.container[cmdLines.indexOUT];
        // Correct ring buffer pointer
        if (++cmdLines.indexOUT >= cmdCount) {
            cmdLines.indexOUT = 0;
        }
        return p;
    } else {
        return 0;
    }
}

void SerialPlcProfile::setBuffer(uint8_t count, char *array) {
    fillNextCommandWithString(array);
}

bool SerialPlcProfile::isBroadcastFrame(char *CurCmd, int16_t idx) {
    enum {
        targetIpStartOffset = 15
    };
    if (idx >= ALIVE_MIN_START_POS) {
        return (CurCmd[31] == 0xFF && CurCmd[32] == 0x02);
    }
    return false;
}

uint16_t SerialPlcProfile::getDestGroupIp(char *CurCmd) {
    enum {
        destShortIpOffset = 45
    };
    return uint8Operations::sum2BytesFromLSB(
            reinterpret_cast<uint8_t*>(CurCmd + destShortIpOffset));
}

}  // namespace AME_SRC
