/*
 * SwitchMatrixAdmin.cpp
 *
 *  Created on: 24.02.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "SwitchMatrixAdmin.h"

#include <cstring>

#include "../../HelpStructures/TriggerOperations.h"
#include "../../SystemBase/ProgramConfig.h"
#include "../../PlcCommunication/PlcTerminalCommandConnector.h"


#define CONFIG_CONTYPE(a) ((a == BOARD_ID) ?  RIGHT : NOTHING)  // check if the command fits to the board ID

namespace AME_SRC {
// PLC command link E_An_Code E_Aus_Code A_Ein_Code A_Aus_Code countDown nextIndex transmissionGroupIndex PLC command
uint16_t SwitchMatrixAdmin::switchMatrix[commandCount][colSize]= {
// *-------------------------------------------Modul Coordinator--------------------------------------------*//
  {       0 ,      RIGHT,     1,         0,        0,        0,        0,         0,             0,       20 },
  {       1 ,      RIGHT,     2,         0,        0,        0,        0,         0,             0,       21 },
 // *-------------------------------------------Modul 1-------------------------------------------- *//
  {       2 ,      RIGHT,     4,         0,        0,        0,        0,         0,             0,       22 },
  {       3 ,      RIGHT,     8,         0,        0,        0,        0,         0,             0,       23 },
 // *-------------------------------------------Modul 2-------------------------------------------- *//
  {       4 ,      RIGHT,     0,         1,        0,         5,        0,         0,            0,       24 },
  {       5 ,      RIGHT,     0,         2,        0,         5,        0,         0,            0,       25 },
 // *-------------------------------------------Modul 3-------------------------------------------- *//
  {       6 ,      RIGHT,     0,         4,        0,         4,        0,         0,            0,       26},
  {       7 ,      RIGHT,     0,         8,        0,         4,        0,         0,            0,       27},
 // *-------------------------------------------Modul 4-------------------------------------------- *//
  {       8,       NOTHING,     1,         0,        4,         3,        0,         0,            1,       9},
  {       9,       NOTHING,        0,         0,        4,         3,        0,         0,            0,       0},
 // *-------------------------------------------Modul 5-------------------------------------------- *//
  {       10,      NOTHING,     1,         0,        5,         2,        0,         0,              1,     11},
  {       11,      NOTHING,        0,         0,        5,         2,        0,         0,            0,       0},
 // *-------------------------------------------Modul 6-------------------------------------------- *//
  {       12,      NOTHING,     1,         0,        6,         1,        0,         0,            1,       13},
  {       13,      NOTHING,        0,         0,        6,         1,        0,         0,            0,       0},
 // *-------------------------------------------Modul 7-------------------------------------------- *//
  {       14,      NOTHING,     1,         0,        7,         0,        0,         0,            1,       15},
  {       15,      NOTHING,        0,         0,        7,         0,        0,         0,            0,       0}
};
// PLC command link E_An_Code E_Aus_Code A_Ein_Code A_Aus_Code countDown nextIndex PLC-command
const uint16_t SwitchMatrixAdmin::connectionMatrix[8][connMemberAmount] =
{{ 0, 0, 0, 0, 0, 0, 0, 0 },
 { 1, 0, 0, 0, 0, 0, 0, 0 },
 { 3, 0, 0, 0, 0, 0, 0, 0 },
 { 4, 0, 0, 0, 0, 0, 0, 0 },
 { 5, 0, 0, 0, 0, 0, 0, 0 },
 { 6, 0, 0, 0, 0, 0, 0, 0 },
 { 7, 0, 0, 0, 0, 0, 0, 0 },
 { 1, 2, 3, 4, 5, 6, 7, 8}};
uint8_t SwitchMatrixAdmin::responseArray[connMemberAmount] = {0 };
const uint16_t (*SwitchMatrixAdmin::signalPollingCommands[commandCount])[colSize] = { 0 };
IoPinGroup *SwitchMatrixAdmin::pinOutGroup = NULL;
IoPinGroup *SwitchMatrixAdmin::pinInGroup = NULL;
int SwitchMatrixAdmin::outstandingSwitchRowIndex = -1;
ArrayStack SwitchMatrixAdmin::validRowNumberStack = ArrayStack();
bool SwitchMatrixAdmin::groupTransmissionCompleteState = true;

SwitchMatrixAdmin::SwitchMatrixAdmin(IoPinGroup *newPinOutGroup,
        IoPinGroup *newPinInGroup, uint16_t boardID,
        IoCommandManager &ioCmdMangerRef) :
        ioCmdMangerRef_ { ioCmdMangerRef } {
    pinOutGroup = newPinOutGroup;
    pinInGroup = newPinInGroup;
    portManager = PortManager::getInstance();
    fillSwitchConnections(boardID);
    fillPollingCommands();
}

void SwitchMatrixAdmin::fillPollingCommands() {
    int pollIndex = 0;
    for (int rowIndex = 0; rowIndex < commandCount; rowIndex++) {
        int connectionCondition = switchMatrix[rowIndex][ConIndex];
        if ((connectionCondition == RIGHT) || (connectionCondition == OR)) {
            signalPollingCommands[pollIndex++] = &switchMatrix[rowIndex];
        }
    }
}

void SwitchMatrixAdmin::fillSwitchConnections(uint16_t boardID) {
    switchMatrix[boardID * 2][ConIndex] = RIGHT;
}

void SwitchMatrixAdmin::executeSwitchWithIndex(uint16_t rowIndex) {
    int connectionCondition = switchMatrix[rowIndex][ConIndex];
    switch (connectionCondition) {
    case (LEFT):  // NO BREAK
    case (OR):
        setDigitalOutputForIndex(rowIndex);
        break;
    case (AND):
        if (calcAndProofSignalForIndex(rowIndex)) {
            setDigitalOutputForIndex(rowIndex);
        }
        break;
    default:
        break;
    }
}

void SwitchMatrixAdmin::setDigitalOutputForIndex(uint16_t index) {
    pinOutGroup->setLevelForBitMask(PortManager::High,
            switchMatrix[index][OutOnIndex]);
    pinOutGroup->setLevelForBitMask(PortManager::Low,
            switchMatrix[index][OutOffIndex]);
}

bool SwitchMatrixAdmin::calcAndProofSignalForIndex(uint16_t index) {
    uint16_t signal = pinInGroup->get2ByteForLevel(PortManager::Low);  // Mutex Aktiv LOW
    return isHighForIndexGiven(signal, index);
}

bool SwitchMatrixAdmin::isHighForIndexGiven(uint16_t signal, uint16_t index) {
    if (signal) {
        bool isHighLevelCorrect =
                (switchMatrix[index][InOnIndex] != 0) ?
                        (signal & switchMatrix[index][InOnIndex]) : false;
        bool isLowLevelCorrect =
                (switchMatrix[index][InOffIndex] != 0) ?
                        ((signal & switchMatrix[index][InOffIndex]) == 0) :
                        true;
        return isHighLevelCorrect & isLowLevelCorrect;
    }
    return false;
}

bool SwitchMatrixAdmin::isLowForIndexGiven(uint16_t signal, uint16_t index) {
    if (signal) {
        bool isHighLevelCorrect =
                (switchMatrix[index][InOnIndex] != 0) ?
                        (((0xF & ~signal) & switchMatrix[index][InOnIndex]) == 0) :
                        true;
        bool isLowLevelCorrect =
                (switchMatrix[index][InOffIndex] != 0) ?
                        (signal & switchMatrix[index][InOffIndex]) : false;
        return isHighLevelCorrect & isLowLevelCorrect;
    }
    return false;
}

void SwitchMatrixAdmin::pollSignalCommandsWithModem() {
    static uint16_t lastSignal = 0;
    uint16_t highSignal = pinInGroup->get2ByteForLevel(PortManager::Low);  // Mutex active LOW

    if (triggerOperations::isNumberChangeForNCalls(highSignal, 2)) {
        if (highSignal != 0) {
          listMatchSignalRowNumbersForSignal(highSignal, 0);
            lastSignal = highSignal;
        } else {
          listMatchSignalRowNumbersForSignal(0, lastSignal);
        }
    }
}

void SwitchMatrixAdmin::sendOutstandingPLCCommandsAndExecuteSignalCalls(ModemInterpreter *modemPtr) {
    static uint8_t waitCount = 4;
    const uint8_t ms200 = 4;
    if (isGroupeTransmissionNotComplete()) {
        if (waitCount++ >= ms200) {
            waitCount = 0;
            uint16_t transmissionIP =
                    switchMatrix[outstandingSwitchRowIndex][transmissionGroupIndex];
            uint16_t cmdNumber =
                    switchMatrix[outstandingSwitchRowIndex][PlcOutIndex];
            ioCmdMangerRef_.set3ParamForIoCmd((uint8_t) cmdNumber,
                    IoCommandManager::fillerParam,
                    IoCommandManager::fillerParam);
            ioCmdMangerRef_.sendIoCmdRequest(transmissionIP,
                    ModemInterpreter::SwitchMatrixRequest,
                    IoCommandManager::noCMD);
            groupTransmissionCompleteState = true;
            // sendSwitchCommandIndexOverModemForConnectionGroupe((uint16_t)outstandingSwitchRowIndex,modemPtr);
        }
    } else {
        if (!validRowNumberStack.isEmpty()) {
            outstandingSwitchRowIndex = validRowNumberStack.pop();
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::txtPopOut),
                    outstandingSwitchRowIndex);
            // ModemInterpreter::modemCrierReff->println(Global2::OutBuff);
            setDigitalOutputForIndex((uint16_t) outstandingSwitchRowIndex);
            groupTransmissionCompleteState = false;
        }
    }
}

void SwitchMatrixAdmin::repeatPLCCommandWithResponseTimeout(
        ModemInterpreter *modemPtr) {
    const uint8_t *responsArrayPtr = responseArray;
    const uint8_t waitTimeout = 6;
    static int responseProofIndex = 0;

    uint8_t responseWaitTime = responsArrayPtr[responseProofIndex];
    if (responseWaitTime > 0) {
        if (responseWaitTime++ > waitTimeout) {
            // ModemInterpreter::modemCrierReff->printFormatMessageWithNumber(
            //      "Fuer Netzwerkteilnehmer %d konnte folgendes Kommando nicht gesendet werden:",responseProofIndex);
            // ModemInterpreter::modemCrierReff->printFormatMessageWithNumber("Kommando M%d",outstandingSwitchRowIndex);
            responseArray[responseProofIndex] = 0;
            return;
        } else if (((responseWaitTime - 1) % 3) == 0) {
            uint16_t tempIdx = (uint16_t) (responseProofIndex + 1);
            if (ModemController::isNetworkMemberIPAktiv(tempIdx)) {
                sendSwitchCommandIndexOverModemForPlcPanAdress(
                        (uint16_t) outstandingSwitchRowIndex, modemPtr);
                // sendSwitchCommandIndexOverModemForPlcPanAdress(outstandingSwitchRowIndex,modemPtr);
                responseArray[responseProofIndex] = responseWaitTime;
            }
          responseArray[responseProofIndex] = responseWaitTime;
        } else {
          responseArray[responseProofIndex] = 0;
        }

      responseArray[responseProofIndex] = responseWaitTime;
    }
    if (++responseProofIndex > ModemController::networkMemberMax) {
        responseProofIndex = 0;
    }
}

bool SwitchMatrixAdmin::isGroupeTransmissionNotComplete() {
    return !groupTransmissionCompleteState;
}

void SwitchMatrixAdmin::sendSwitchCommandIndexOverModemForConnectionGroupe(
        uint16_t comIndex, ModemInterpreter *modemPtr) {
    static uint16_t groupeColOffset = 0;
    uint16_t connectionIndex = switchMatrix[comIndex][transmissionGroupIndex];
    sendSwitchCommandIndexOverModemForPlcPanAdress(comIndex, modemPtr);
    groupTransmissionCompleteState = true;
//      while(connectionMatrix[connectionIndex][groupeColOffset] == 0){
//          groupeColOffset++;
//      }
//      if ((groupeColOffset > connMemberAmount)) {
//          groupTransmissionCompleteState = true;
//          groupeColOffset = 0;
//          return;
//      }
//      uint16_t panAdress = connectionMatrix[connectionIndex][groupeColOffset];
//      if(panAdress > 0 && ModemController::isNetworkMemberIPAktiv( -1)){
//          sendSwitchCommandIndexOverModemForPlcPanAdress(comIndex,modemPtr,panAdress);
//      }
//      groupeColOffset++;
}

void SwitchMatrixAdmin::listMatchSignalRowNumbersForSignal(uint16_t highSignal, uint16_t lowSignal) {
    for (int i = 0; i < commandCount; i++) {
        const uint16_t (*commandPtr)[colSize] = signalPollingCommands[i];
        if (commandPtr != 0) {
            int index = (*signalPollingCommands[i])[PlcInIndex];
            bool isInput = switchMatrix[i][ConIndex] == RIGHT;
            if (isInput
                    && (isHighForIndexGiven(highSignal, index)
                            || isLowForIndexGiven(lowSignal, index))) {
                validRowNumberStack.push(index);
            }
        }
    }
}

void SwitchMatrixAdmin::sendSwitchCommandIndexOverModemForPlcPanAdress(
        uint16_t comIndex, ModemInterpreter *modemPtr) {
    uint16_t plcCommand = switchMatrix[comIndex][PlcOutIndex];
    if (plcCommand != 0) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::switchMatrixSendMsg),
                IRoutingManager::BCGlobalIP  /*ipAdress*/,
                modemPtr->commandPrefix, plcCommand,
                 ModemController::getNetworkIPAddress());
        PlcTerminalCommandConnector::sendMessageToNetworkMemberIndex(
                Global2::OutBuff);
    }
}

SwitchMatrixAdmin::~SwitchMatrixAdmin() {
    // TODO(AME) Auto-generated destructor stub
}

}  // namespace AME_SRC
