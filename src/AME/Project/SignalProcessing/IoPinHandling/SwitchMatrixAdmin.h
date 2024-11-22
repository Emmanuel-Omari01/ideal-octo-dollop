/*
 * SwitchMatrixAdmin.h
 *
 *  Created on: 24.02.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_SWITCHMATRIXADMIN_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_SWITCHMATRIXADMIN_H_

#include "PortManager.h"
#include "../../PlcCommunication/ModemInterpreter.h"
#include "../../PlcCommunication/ModemController.h"
#include "../../HelpStructures/ArrayStack.h"
#include "IoPinGroup.h"

namespace AME_SRC {

class SwitchMatrixAdmin {
    static const uint16_t commandCount = 16;
    static const uint16_t colSize = 10;
    static uint16_t switchMatrix[commandCount][colSize];
    static const uint16_t (*signalPollingCommands[commandCount])[colSize];
    static const uint16_t connMemberAmount = 8;
    static const uint16_t connectionMatrix[8][connMemberAmount];
    static int outstandingSwitchRowIndex;
    PortManager *portManager;
    static IoPinGroup *pinOutGroup;
    static IoPinGroup *pinInGroup;
    static uint8_t responseArray[connMemberAmount];
    static ArrayStack validRowNumberStack;
    static bool groupTransmissionCompleteState;

 public:
    SwitchMatrixAdmin(IoPinGroup *pinOutGroup, IoPinGroup *pinInGroup,
            uint16_t boardID, IoCommandManager &ioCmdMangerRef);
    static void executeSwitchWithIndex(uint16_t index);
    static void sendSwitchCommandIndexOverModemForPlcPanAdress(
            uint16_t comIndex, ModemInterpreter *modemPtr);
    static void sendSwitchCommandIndexOverModemForConnectionGroupe(
            uint16_t comIndex, ModemInterpreter *modemPtr);
    static void pollSignalCommandsWithModem();
    void sendOutstandingPLCCommandsAndExecuteSignalCalls(
            ModemInterpreter *modemPtr);
    static void repeatPLCCommandWithResponseTimeout(ModemInterpreter *modemPtr);
    static bool isGroupeTransmissionNotComplete();
    virtual ~SwitchMatrixAdmin();

    static void setAckForIndex(uint8_t index) {
      responseArray[index] = 0;
    }

 private:
    IoCommandManager &ioCmdMangerRef_;
    enum conCondition {
        OR = 0, AND, LEFT, RIGHT, NOTHING
    };
    enum switchColDescription {
        PlcInIndex = 0,
        ConIndex,
        InOnIndex,
        InOffIndex,
        OutOnIndex,
        OutOffIndex,
        CountDownIndex,
        NextIndex,
        transmissionGroupIndex,
        PlcOutIndex
    };
    void fillPollingCommands();
    void fillSwitchConnections(uint16_t boardID);
    static bool isHighForIndexGiven(uint16_t signal, uint16_t index);
    static bool isLowForIndexGiven(uint16_t signal, uint16_t index);
    static bool calcAndProofSignalForIndex(uint16_t index);
    static void setDigitalOutputForIndex(uint16_t index);
    static void listMatchSignalRowNumbersForSignal(uint16_t highSignal, uint16_t lowSignal);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_SWITCHMATRIXADMIN_H_
