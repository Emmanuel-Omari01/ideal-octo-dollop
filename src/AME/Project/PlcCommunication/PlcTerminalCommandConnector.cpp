/*
 * PlcTerminalCommandConnector.cpp
 *
 *  Created on: 10.06.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "PlcTerminalCommandConnector.h"

#include <cstdlib>

#include "ModemInterpreter.h"
#include "../HelpStructures/CharOperations.h"
#include "../SignalProcessing/I2C/I2cApp.h"
#include "G3PlcSystem/RequestFactory/RequestFactory.h"
#include "AliveHandling/MessageForwarding/ForwardingFlow.h"
#include "AliveHandling/NeighbourAssignment/AssignmentFlow.h"
#include "../StorageManagement/SimpleFlashEditor.h"
#include "../SystemBase/ConfigTermConnector.h"


namespace AME_SRC {

uint8_t PlcTerminalCommandConnector::RequestArgData[16];
#ifdef USE_VERBOSE_FLAGS
uint32_t PlcTerminalCommandConnector::verboseMask = verboseL2|
                                                    verbosePLCData|
                                                    verboseAliveRxInfo|
                                                    verboseAliveTxInfo|
                                                    verbosePLCText|
                                                    verboseJoinTabs|
                                                    verboseInitLoadBar|
                                                    verboseRoutingInfo|
                                                    verboseAliveJumpInfo|
                                                    verboseIntegrationTest;
#endif

PlcTerminalCommandConnector::PlcTerminalCommandConnector() = default;

PlcTerminalCommandConnector::~PlcTerminalCommandConnector() = default;

const AME_SRC::termCommand commandTab[] = {
    { NULL, NULL, NULL, TX::txtPlcCmd, TX::dTxtSeparator},
    {NULL, NULL, PlcTerminalCommandConnector::leave, TX::CmdPlCLeave, TX::DescriptPLCLeave},
    {NULL, NULL, PlcTerminalCommandConnector::callInitChannelWithMode, TX::cTxtPlcInit, TX::dTxtPlcInit},
    {NULL, NULL, PlcTerminalCommandConnector::join, TX::cTxtPlcJoin, TX::dTxtPlcJoin},
    {NULL, NULL, PlcTerminalCommandConnector::forcejoin, TX::cTxtPlcFJoin, TX::dTxtPlcFJoin},
    {NULL, NULL, PlcTerminalCommandConnector::forceNetworkMember, TX::cTxtPlcFNetMember, TX::dTxtPlcFNetMember},
    {NULL, NULL, PlcTerminalCommandConnector::searchNetwork, TX::cTxtSearchNet, TX::dTxtSearchNet},
    {NULL, NULL, PlcTerminalCommandConnector::getMacAttribute, TX::cTxtGetMAC, TX::dTxtGet},
    {NULL, NULL, PlcTerminalCommandConnector::setMacAttribute, TX::cTxtSetMAC, TX::dTxtSet},
    {NULL, NULL, PlcTerminalCommandConnector::getADPAttribute, TX::cTxtGetADP, TX::dTxtGet},
    {NULL, NULL, PlcTerminalCommandConnector::setADPAttribute, TX::cTxtSetADP, TX::dTxtSet},
    {NULL, NULL, PlcTerminalCommandConnector::getEAPAttribute, TX::cTxtGetEAP, TX::dTxtGet},
    {NULL, NULL, PlcTerminalCommandConnector::setEAPAttribute, TX::cTxtSetEAP, TX::dTxtSet},
    {NULL, NULL, PlcTerminalCommandConnector::setMacAttribute, TX::cTxtSetMAC, TX::dTxtSet},
    {NULL, NULL, PlcTerminalCommandConnector::setClientInfo, TX::cTxtSetClient, TX::dTxtSetClient},
    {NULL, NULL, PlcTerminalCommandConnector::getSystemVersionCmd, TX::cTxtGetSysVersion, TX::dTxtGetSysVersion},
    { NULL, NULL, ModemController::showNetworkMembers,                  TX::cTxtShowMem,       TX::dTxtShowMem},
    {NULL, NULL, PlcTerminalCommandConnector::sendMessageToNetworkMemberIndex, TX::cTxtSendMess, TX::dTxtSendMess},
    {NULL, NULL, ConfigTermConnector::termSendPLCRawData, TX::CmdSendPLCRaw , TX::DescriptSendRaw},
    {NULL, NULL, PlcTerminalCommandConnector::setVerboseTerminal, TX::cTxtSetVerb, TX::dTxtSetVerb},
    {NULL, NULL, PlcTerminalCommandConnector::pathDiscovery, TX::cTxtPathDisc, TX::dTxtPathDisc},
    //{ NULL, NULL, PlcTerminalCommandConnector::remotePathDiscovery,   TX::cTxtRPathDisc,  TX::dTxtRPathDisc},
    {NULL, NULL, PlcTerminalCommandConnector::startAliveCheck, TX::cTxtStartAlive, TX::dTxtStartAlive},
    { NULL, NULL, AssignmentFlow::broadcastGroupSelection,              TX::cTxtAliveNPeer, TX::dTxtAliveNPeer},
    {NULL, NULL, PlcTerminalCommandConnector::remoteSearchNetwork, TX::cTxtRSearch, TX::dTxtRSearch},
    { NULL, NULL, AssignmentFlow::triggerShowNetworkDiscoveryResults,   TX::cTxtRNetShow,   TX::dTxtRNetShow},
    {NULL, NULL, PlcTerminalCommandConnector::triggerAutomaticSteps, TX::CmdTriggerAutomatic, TX::DescriptTriggerAuto},
    {NULL, NULL, PlcTerminalCommandConnector::terminalStartCmd, TX::CmdStart, TX::DescriptExecAlive},
    { NULL, NULL, NULL, TX::CmdStart, TX::DescriptIntegrationTest},
    {NULL, NULL, PlcTerminalCommandConnector::terminalStopCmd, TX::CmdStop, TX::DescriptStopAliveTest},
    { NULL, NULL, NULL, TX::CmdStop, TX::DescriptStopIntegrationTest},
    { NULL, NULL, AssignmentFlow::terminalSetFlowSteps, TX::CmdAutoSubSteps, TX::DescriptAutoSubStep },
    {NULL, NULL, PlcTerminalCommandConnector::lineEntryTestFkn, TX::CmdLineEntryTest, TX::DescriptionLineEntryTest},
    {NULL, NULL, PlcTerminalCommandConnector::blockStoreTable, TX::CmdBlockStoreTable, TX::DescriptBlockStoreTable},
    {NULL, NULL, PlcTerminalCommandConnector::getPLCAttributesStart, TX::cTxtGetPLCAttr, TX::dTxtGetPLCAttr},
    { NULL, NULL, AssignmentFlow::remoteBroadcastGroupSelection, TX::CmdBCGroupSet, TX::DescriptBCGroupSet},
    {NULL, "", NULL, TX::dTxtSeparator, -1},
    { NULL, "",   NULL, TX::dTxtFJCoordMsg, -1},
    {NULL, NULL, PlcTerminalCommandConnector::setFCCoordCallOuts, TX::cTxtFJCoordCallOuts, TX::dTxtFJCoordCallOuts},
    {NULL, NULL, PlcTerminalCommandConnector::setFCCoordDelay, TX::cTxtFJCoordDelay, TX::dTxtFJCoordDelay},
    {NULL, NULL, PlcTerminalCommandConnector::setFCCoordScale, TX::cTxtFJCoordScale, TX::dTxtFJCoordScale},
    { NULL, NULL, AutostartFlow::setFJIgnoreRequests, TX::CmdIgnoreFCRequests, TX::DescriptFCRequest},
    {NULL, NULL, PlcTerminalCommandConnector::testMalloc, TX::CmdTestMalloc, TX::DescriptionTestMalloc},
    { NULL, NULL, AssignmentFlow::fillAdjMatrixTerminal, TX::CmdFillAdjMatrix, TX::DescriptFillAdjMatrix},
    { NULL, NULL, AssignmentFlow::showMatrixOnTerminal,  TX::CmdShowAdjMatrix, TX::DescriptShowAdjMatrix},
    { NULL, NULL, AssignmentFlow::presetAliveRoute, TX::CmdPresetAliveRoute,   TX::DescriptPresetAliveRoute},
    {NULL, NULL, PlcTerminalCommandConnector::registerVirtualPeers, TX::CmdRegVirtPeers, TX::DescriptRegVirtPeers},
    {NULL, NULL, PlcTerminalCommandConnector::clearDeviceFC, TX::CmdClearFC, TX::DescriptClearFC},
    { NULL, NULL, ForwardingFlow::startStopAliveRoundTrips, TX::CmdEnableAliveRound, TX::DescriptEnableAlive},
    {NULL, NULL, PlcTerminalCommandConnector::pollPeer, TX::CmdPoll, TX::DescriptPoll},
    {NULL, NULL, PlcTerminalCommandConnector::inputPeer, TX::CmdInput, TX::DescriptInput},
    {NULL, NULL, PlcTerminalCommandConnector::outputPeer, TX::CmdOutput, TX::DescriptOutput},
    {NULL, NULL, PlcTerminalCommandConnector::sendGlobalTimeout, TX::CmdSendGlobalTimeout, TX:: DescriptGlobalTimout},
    { NULL, NULL, i2c_app::i2c_sample_read, TX::CmdI2cRead, TX::DescriptI2cRead},
    { NULL, NULL, i2c_app::i2c_sample_write, TX::CmdI2cWrite, TX::DescriptI2cWrite},
    { NULL, NULL, i2c_app::i2c_init, TX::CmdI2cInit, TX::DescriptI2cInit},
    { NULL, NULL, i2c_app::i2cMacRead, TX::CmdI2RdMac, TX::DescriptI2RdMac},
    { reinterpret_cast<char *>(0xFFFFFFFF), NULL, NULL, 0, 0 },  // end of table (typedef endList)
};

void PlcTerminalCommandConnector::addCmdTab2List() {
    CommandInterpreter::cmdListTab.add((void *) commandTab);
}

/***
void PlcTerminalCommandConnector::addCommandsToList(
		List<Command>& cmdList) {
	//cmdList.add(resetModemCmd);
//	cmdList.add(plcSetAdressCmd);
//	cmdList.add(plcSendCmd);
//	cmdList.add(setAdressCmd);
//	cmdList.add(getAdressCmd);
//	cmdList.add(kickMemberCmd);
//	cmdList.add(pingStartCmd);
//	cmdList.add(pingStopCmd);
}
***/

char* PlcTerminalCommandConnector::sendMessageToNetworkMemberIndex(char *text) {
    unsigned int targetIP;
    char broadCastOpt;
    const char *txt = charOperations::skip_token(text, ' ');

    int32_t cnt = sscanf(txt, "%c %u %100s", &broadCastOpt, &targetIP,
            Global2::InBuff);
    if (cnt == 3) {
        txt = charOperations::skip_token(txt, ' ');
        txt = charOperations::skip_token(txt, ' ');
        if (broadCastOpt == 'B') {
            ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                    reinterpret_cast<uint8_t*>(Global2::InBuff),
                    (uint8_t) strlen(Global2::InBuff), (uint16_t) targetIP,
                    true, TermContextFilter::filterPLCData | TermContextFilter::filterPLCText);
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtSendMsgToMember),
                    Global2::InBuff, targetIP);
        } else {
            NetworkMember *selectedMemberPtr =
                    ModemController::getNetMemberPtrByIP((uint16_t) targetIP);
            if (selectedMemberPtr != NULL
                    && selectedMemberPtr->connectedOrNotVerified()) {
                ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(
                        const_cast<char*>(txt), (uint8_t) targetIP, false);
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtSendMsgToMember),
                        txt, targetIP);
            } else {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s",
                        TermContextFilter::isFiltered(TermContextFilter::filterPLCText)?
                                "" : TX::getText(TX::dTxtInactivity));
            }
        }
    } else {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::InvalidNumberArguments));
    }

    return Global2::OutBuff;
}


char* PlcTerminalCommandConnector::pathDiscovery(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    if (first2Integers.val1 <= ModemController::networkMemberMax) {
        // Look at PLC_Specification 6.10 ADPM-PATH-DISCOVERY
        ModemController::triggerSinglePlcRequestWithArgs(
                PLC_CommandTypeSpecification::pathDiscovery, 3, channel0,
                first2Integers.val1, first2Integers.val2);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtStartDiscovery),
                first2Integers.val1);
        return Global2::OutBuff;
    } else {
        return const_cast<char*>(TX::getText(TX::dTxtIDNotKnown));
    }
}

// char *PlcTerminalCommandConnector::remotePathDiscovery(char *text) {
//    containerFor2Values first2Integers;
//    getFirst2IntegerFromString(text, &first2Integers);
//      sprintf(Global2::OutBuff,"%s%c%03d",ModemInterpreter::commandPrefix,ModemInterpreter::PathDiscoveryRequest,first2Integers.val2);
//      ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(Global2::OutBuff,
//                          (uint8_t) first2Integers.val1, false);
//      return (char*)TX::getText(TX::dTxtRemotePathDiscov);
//  } else {
//      return (char*)TX::getText(TX::dTxtIDNotKnown);
//  }
//}


char* PlcTerminalCommandConnector::getEAPAttribute(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::getEapAttribute, 3, channel0,
            first2Integers.val1, first2Integers.val2);
    return const_cast<char*>(TX::getText(TX::cTxtEAPAttrRequest));
}


char* PlcTerminalCommandConnector::getMacAttribute(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::getMacAttribute, 3, channel0,
            first2Integers.val1, first2Integers.val2);
    return const_cast<char*>(TX::getText(TX::cTxtMACAttrRequest));
}

char* PlcTerminalCommandConnector::getADPAttribute(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::getAdpAttribute, 3, channel0,
            first2Integers.val1, first2Integers.val2);
    return const_cast<char*>(TX::getText(TX::cTxtADPAttrRequest));
}


char* PlcTerminalCommandConnector::setMacAttribute(char *text) {
    ModemController::getRequestArgs getArgs;
    getRequestArgsFromString(text, &getArgs);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::setMacAttribute, 4, channel0,
            getArgs.identifier, getArgs.tableIndex, (uint8_t) getArgs.dataLen,
            (uint32_t) getArgs.dataPtr);
    return const_cast<char*>(TX::getText(TX::cTxtMACAttrSet));
}

// Set ADP-Route
// Example for terminal input: setADPAttribute 12 00 000000007FFF08080100
char* PlcTerminalCommandConnector::setADPAttribute(char *text) {
    ModemController::getRequestArgs getArgs;
    getRequestArgsFromString(text, &getArgs);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::setAdpAttribute, 4, channel0,
            getArgs.identifier, getArgs.tableIndex, (uint8_t) getArgs.dataLen,
            (uint32_t) getArgs.dataPtr);
    return const_cast<char*>(TX::getText(TX::cTxtADPAttrSet));
}

char* PlcTerminalCommandConnector::setEAPAttribute(char *text) {
    ModemController::getRequestArgs getArgs;
    getRequestArgsFromString(text, &getArgs);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::setEapAttribute, 4, channel0,
            getArgs.identifier, getArgs.tableIndex, (uint8_t) getArgs.dataLen,
            (uint32_t) getArgs.dataPtr);
    return const_cast<char*>(TX::getText(TX::cTxtEAPAttrSet));
}

char* PlcTerminalCommandConnector::setClientInfo(char *text) {
    char *textPtr, *savePtr;
    bool validFlag;
    static uint8_t extendedAddress[8];
    uint16_t networkAddress;
    textPtr = strtok_r(text, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    validFlag = static_cast<bool>(atoi(textPtr));
    textPtr = strtok_r(NULL, " ", &savePtr);
    // memcpy(extendedAddress,textPtr,8);
    for (int i = 0; i < 8; i++) {
        extendedAddress[i] = uint8Operations::sum2HexChars(textPtr + (i * 2));
    }
    textPtr = strtok_r(NULL, " ", &savePtr);
    networkAddress = (uint16_t) atoi(textPtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::setClientInfo, 2, channel0, validFlag,
            networkAddress, (uint32_t) extendedAddress);
    return const_cast<char*>((TX::getText(TX::cTxtClientAttrSet)));
}

char* PlcTerminalCommandConnector::getSystemVersionCmd(__attribute__((unused)) char *dummy) {
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::getSystemVersion, 1, channel0);
    return const_cast<char*>(TX::getText(TX::dTxtGetSysVersion));
}

char* PlcTerminalCommandConnector::callInitChannelWithMode(char *mode) {
    char *modePtr, *savePtr;
    modePtr = strtok_r(mode, " ", &savePtr);
    modePtr = strtok_r(NULL, " ", &savePtr);

    if (*modePtr == 'P') {
        ProgramConfig::setCoordinatorStatus(false);
    } else if (*modePtr == 'C') {
        ProgramConfig::setCoordinatorStatus(true);
    } else {
        return const_cast<char*>(TX::getText(TX::dTxtPorCEntry));
    }
    ModemController::setProcessState(ModemController::createInitPlcReqListForChannel(channel0).getHead());

    return const_cast<char*>(TX::getText(TX::dTxtG3ChannelInit));
}

char* PlcTerminalCommandConnector::setVerboseTerminal(char *text) {
    unsigned int cnt, mask, actMask;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%x", &mask);
    if (cnt == 1) {
        actMask = TermContextFilter::getFilterMask();
        if (mask <= TermContextFilter::filterL2) {
            actMask &= ~(TermContextFilter::filterL1 | TermContextFilter::filterL2);  // delete
            actMask |= mask;                      // set
        } else {
            actMask = mask;
        }
        TermContextFilter::setFilterMask((TermContextFilter::filterBits) actMask);
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s: %x", TX::getText(TX::dTxtVerboseSet),
            (unsigned int) TermContextFilter::getFilterMask());
    return Global2::OutBuff;
}


char* PlcTerminalCommandConnector::join(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::joinNetwork, 3, channel0,
            first2Integers.val1, first2Integers.val2);
    return const_cast<char*>(TX::getText(TX::dTxtJoinTriggered));
}

char* PlcTerminalCommandConnector::leave(__attribute__((unused)) char *text) {
    ModemController::triggerSinglePlcRequestWithArgs(leaveNetwork, 1, channel0);
    return const_cast<char*>(TX::getText(TX::LeaveNetworkCmdMsg));
}

char* PlcTerminalCommandConnector::forcejoin(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::forceJoinNetwork, 3, channel0,
            first2Integers.val1, first2Integers.val2);
    return const_cast<char*>(TX::getText(TX::dTxtForceJoinTrigger));
}


char* PlcTerminalCommandConnector::triggerAutomaticSteps(char *text) {
    AssignmentFlow &f = ProgramConfig::getPlcModemPtr()->getAssigntmentFlowRef();
    return f.triggerAutomaticSteps(text);
}


/***
 * Short: Terminal function for starting the alive check
 * Terminal function for starting the alive check on coordinator side.
 * The Method proofs if the current module is coordinator and set the
 * aliveCheck status insde the ModemController
 * @param text  aliveCheck Status
 * @return Answer on the input Parameter which is displayed on the user Terminal
 */
char* PlcTerminalCommandConnector::startAliveCheck(char *text) {
    if (!ProgramConfig::isCoordinator()) {
        return const_cast<char*>(TX::getText(TX::dTxtAliveCheckBlock));
    } else {
        char temp[20];
        unsigned int val1, val2;
        int32_t cnt;
        temp[0] = 0;
        const char *txt = charOperations::skip_token(text, ' ');
        cnt = sscanf(txt, "%u %u", &val1, &val2);
        if (cnt == 2) {
            if (val1 <= 1) {
                ModemController::setAliveCheckEnabled(val1);
                if (val1) {
                    ModemController::setAliveCheckLoops((uint16_t) val2);
                    snprintf(temp, sizeof(temp), TX::getText(TX::LoopsMsg) , val2);
                }
            }
        } else if (cnt == 1) {
            if (val1 <= 1) {
                ModemController::setAliveCheckEnabled(val1);
            }
            ModemController::setAliveCheckLoops(0);
        }
        ProgramConfig::setAliveTimeJobPtr(&TimeMeasurement::registerTimedJob(-1));
        ProgramConfig::setAliveRoundJobPtr(&TimeMeasurement::registerTimedJob(-1));
        ModemController::setAliveTimeOutJob(&TimeMeasurement::registerTimedJob(-1));
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::AliveTestEnabled),
                ModemController::isAliveCheckEnabled()? "":TX::getText(TX::txtNot), temp);
        return Global2::OutBuff;
    }
}




char* PlcTerminalCommandConnector::forceNetworkMember(char *text) {
    unsigned int index, ipAdr, macAdr, lnkState;
    const char *txt = charOperations::skip_token(text, ' ');
    int32_t cnt = sscanf(txt, "%u %u %u %u", &index, &ipAdr, &macAdr,
            &lnkState);
    bool lFail = true;
    NetworkMember *netMemberPtr = NULL;

    if (cnt >= 2) {
        netMemberPtr = ModemController::getNetMemberPtrByIndex(
                (uint16_t) index);
        lFail = (netMemberPtr == NULL)
                || (ipAdr >= ModemController::networkMemberMax);
        if (!lFail) {
            netMemberPtr->setPendingTime(10);
          netMemberPtr->setNetworkIPAddress((uint16_t) ipAdr);
        }
    }
    if (!lFail && cnt >= 3) {
        lFail = macAdr >= 0x10000;
        if (!lFail) {
          netMemberPtr->setMacShortAddress((uint16_t) macAdr);
        }
    }

    if (!lFail && cnt >= 4) {
        NetworkMember::linkStatus max = NetworkMember::maxEntry;
        lFail = lnkState > max;
        if (!lFail) {
            netMemberPtr->setLink((NetworkMember::linkStatus) lnkState);
        }
    }

    if (lFail) {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    } else {
        return const_cast<char*>(TX::getText(TX::dTxtForceNetMemberSet));
    }
}



char* PlcTerminalCommandConnector::searchNetwork(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::searchNetwork, 2, channel0,
            first2Integers.val1);
    return const_cast<char*>(TX::getText(TX::cTxtNetSearchStarted));
}


void PlcTerminalCommandConnector::getFirst2IntegerFromString(char *str, containerFor2Values *container) {
    char *textPtr, *savePtr;
    textPtr = strtok_r(str, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    container->val1 = (uint16_t) atoi(textPtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    container->val2 = (uint16_t) atoi(textPtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    str = textPtr;
}

PlcTerminalCommandConnector::containerFor2Values* PlcTerminalCommandConnector::create2IntContainerFromPLCString(
        const char *str) {
    unsigned int tmp1, tmp2;
    const char *textPtr = skipPlcCmdHeader(str);
    sscanf(textPtr, "%03u:%03u", &tmp1, &tmp2);
    containerFor2Values *containerPtr = new containerFor2Values;
    containerPtr->val1 = (uint16_t) tmp1;
    containerPtr->val2 = (uint16_t) tmp2;
    return containerPtr;
}

const char* PlcTerminalCommandConnector::skipPlcCmdHeader(const char *str) {
    const char *textPtr = str;
    for (int i = 0; i < 3; i++) {
        textPtr = charOperations::skip_token(textPtr, ':');
    }
    return textPtr;
}


char* PlcTerminalCommandConnector::lineEntryTestFkn(char *text) {
    containerFor2Values first2Integers;
    getFirst2IntegerFromString(text, &first2Integers);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::TestFunctionNr), first2Integers.val1);
    switch (first2Integers.val1) {
    case 1:
        ModemController::setProcessState(
                ModemController::createSetForceJoinInfo(ModemController::start).getHead());
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::TestRestoreJoinTable));
        break;
    case 2:
        ModemController::restoreMemberList();
        break;
    case 3:
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s",
                TX::getText((I18N::textNumber) first2Integers.val2));
        break;
    }

    return Global2::OutBuff;

    /*** test input of an Byte-Arrays per terminal
     char *textPtr;
     textPtr = strtok(text, " ");
     textPtr = strtok(NULL, " ");
     int8_t effLen;
     bool lOk = uint8Operations::str2binArray((uint8_t*)textPtr, RequestArgData, sizeof(RequestArgData), &effLen);

     sprintf(Global2::OutBuff,"Eingabe-Text, Laenge: %d, Ok: %d",effLen,lOk);
     return Global2::OutBuff;
     ***/
}

char* PlcTerminalCommandConnector::remoteSearchNetwork(char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    if (*txt == 'S') {
        return AssignmentFlow::triggerNetworkDiscovery('S', text);
    } else {
        return AssignmentFlow::triggerNetworkDiscovery('T', text);
    }
}


char* PlcTerminalCommandConnector::clearDeviceFC(char *text) {
    int srcAddr, tableOffset;
    const char *txt = charOperations::skip_token(text, ' ');
    int32_t cnt = sscanf(txt, "%d %d", &tableOffset, &srcAddr);
    if (cnt == 2) {
        ModemController::triggerSinglePlcRequestWithArgs(
                RequestFactory::createUserSetRequest(ClearDeviceFrameCounter, 3, tableOffset, srcAddr));
        return const_cast<char*>(TX::getText(TX::ArgumentIsStored));
    }
    return const_cast<char*>(TX::getText(TX::InvalidNumberArguments));
}

void PlcTerminalCommandConnector::getRequestArgsFromString(char *str, ModemController::getRequestArgs *container) {
    char *textPtr, *savePtr;
    int8_t effLen;
    textPtr = strtok_r(str, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    container->identifier = (uint16_t) atoi(textPtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    container->tableIndex = (uint16_t) atoi(textPtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    uint8Operations::str2binArray(reinterpret_cast<uint8_t*>(textPtr), RequestArgData, sizeof(RequestArgData), &effLen);
    container->dataPtr = RequestArgData;
    container->dataLen = effLen;
}

// Suppress or enable the storing of routing information in memory
char* PlcTerminalCommandConnector::blockStoreTable(char *text) {
    unsigned int disable;
    int32_t cnt;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u", &disable);
    if (cnt >= 1) {
        ProgramConfig::setBlockStoreFlashTable(static_cast<bool>(disable));
    } else {
        disable = (uint32_t) ProgramConfig::getBlockStoreFlashTable();
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::BlockStoreTableState) , disable);
    return Global2::OutBuff;
}

char* PlcTerminalCommandConnector::getPLCAttributesStart(__attribute__((unused)) char *text) {
    ModemController::setProcessState(
            ModemController::createAttributeDump(true).getHead());
    return const_cast<char*>(TX::getText(TX::dTxtGetPLCAttrConfirm));
}

char* PlcTerminalCommandConnector::setFCCoordCallOuts(char *text) {
    unsigned int val1;
    int32_t cnt;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u", &val1);
    if (cnt >= 1) {
        SimpleFlashEditor::setFJCallout((uint16_t) val1);
    } else {
        val1 = SimpleFlashEditor::getFJCallout();
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtFJCoordCallOutSet), val1);
    return Global2::OutBuff;
}

char* PlcTerminalCommandConnector::setFCCoordDelay(char *text) {
    unsigned int val1;
    int32_t cnt, index = 0;
    char cWhat;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%c %u", &cWhat, &val1);
    if (cnt >= 1) {
        switch (cWhat) {
        default:
            cnt = 1;
            break;
        case 'X':
        case 'Y':
        case 'Z':
            index = cWhat - 'X';
            break;
        }
        if (cnt == 2 && val1 > 255) {
            cnt = 1;
        }
    }
    if (cnt == 2) {
        SimpleFlashEditor::setFJDelay((uint8_t) val1, (uint16_t) index);
    } else {
        val1 = SimpleFlashEditor::getFJDelay((uint16_t) index);
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtFJCoordDelaySet), index + 'X', val1);
    return Global2::OutBuff;
}

char* PlcTerminalCommandConnector::setFCCoordScale(char *text) {
    unsigned int val1;
    int32_t cnt;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u", &val1);
    if (cnt >= 1) {
        SimpleFlashEditor::setFJScale((uint16_t) val1);
    } else {
        val1 = SimpleFlashEditor::getFJScale();
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtFJCoordScaleSet), val1);
    return Global2::OutBuff;
}


/****
Function testMalloc.
The function checks whether it is possible to allocate a heap memory of x bytes.
If it is possible, a positive message is output on the terminal and the allocated memory is immediately released again.
If this is not possible, a negative message will be displayed on the terminal.
The function must be used iteratively until a negative message appears
****/
char* PlcTerminalCommandConnector::testMalloc(char *text) {
    int cnt, memory, release;
    void *pHeap;

    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u %u", &memory, &release);
    if (cnt >= 1) {
        pHeap = malloc(memory);

        if (pHeap == NULL || ((uint32_t) pHeap) >= 0x10000) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::MallocFailMsg) , pHeap);
            // Hints:
            // - the pointer should not be used if an address range that is outside of RAM is returned:
            //   The RX1xx has a RAM area of 0-0x10000 = 64 kBytes
            //   When using the pHeap vector incorrectly, I (DS) even managed to overwrite the modem's boot ROM.
            //   It is unclear how this is possible. The boot rom had to be reinitialized.
            // - When testing at this stage of development, it was possible to use this function to create a memory
            //   block of 920 bytes to request. When the program starts, the heap is initialized with
            //   0x1000 = 4096 bytes. So there is only about 22% of heap memory available.
            //  This should be done in the further development of the Software should be taken into account.
        } else {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::MallocAvailableMsg), memory);
            if (cnt == 1)
                release = 1;
            if (release) {
                free(pHeap);
            }
        }
    } else {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "?");
    }
    return Global2::OutBuff;
}  // testMalloc


char* PlcTerminalCommandConnector::registerVirtualPeers(char *text) {
    unsigned int val1;
    int32_t cnt;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u", &val1);
    if (cnt < 1) {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    }
    ModemController::activateTestMembersUptoIP((uint16_t) val1);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::VirtualPeersAreRegistered), val1);
    return Global2::OutBuff;
}

char* PlcTerminalCommandConnector::pollPeer(char *text) {
    unsigned int val1;
    char val2[16];
    int32_t cnt;
    const char *txt = charOperations::skip_token(text, ' ');
    char *message;
    cnt = sscanf(txt, "%u %15s", &val1, val2);
    // cnt = sscanf(txt, "%u %s", &val1, val2);
    if (cnt < 1) {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    } else if (cnt == 1) {
        IoCommandManager::setPollRequestState(IoCommandManager::startPoll);
    } else {
        char *ret;
        ret = strstr(val2, TX::getText(TX::CmdStop));
        if (ret) {
            IoCommandManager::setPollRequestState(IoCommandManager::stopPoll);
        } else {
            ret = strstr(val2, TX::getText(TX::CmdVerify));
            if (ret) {
                IoCommandManager::setPollRequestState(IoCommandManager::verifyPoll);
            } else {
                return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
            }
        }
    }
    message = sendIoCmd2Peer((uint16_t) val1, ModemInterpreter::PollRequest,
            IoCommandManager::getPollRequestState());
    return message;
}

char* PlcTerminalCommandConnector::inputPeer(char *text) {
    int32_t cnt;
    unsigned int tmp1, tmp2;
    const char *txt = charOperations::skip_token(text, ' ');
    char *message;
    cnt = sscanf(txt, "%u %u", &tmp1, &tmp2);
    // containerFor2Values first2Integers = getFirst2IntegerFromString(text);
    if (cnt < 1) {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    } else if (cnt > 1) {
        IoCommandManager::setInputDisplayIndex((uint8_t) tmp2);
    } else {
        IoCommandManager::setInputDisplayIndex(0xFF);
    }
    message = sendIoCmd2Peer((uint16_t) tmp1, ModemInterpreter::InputRequest, IoCommandManager::noCMD);
    return message;
}


char* PlcTerminalCommandConnector::sendIoCmd2Peer(uint16_t targetIP,
                                                  const char cmd, enum IoCommandManager::ioSubCmds subCmd) {
    if (targetIP == 0) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::CoordinatorMayNotRequested));
    } else {
        IoCommandManager::sendIoCmdRequest(targetIP, cmd, subCmd);
        Global2::OutBuff[0] = 0;
    }
    return Global2::OutBuff;
}

char* PlcTerminalCommandConnector::outputPeer(char *text) {
    int32_t cnt;
    unsigned int tmp1, tmp2, tmp3;
    enum { fillerParam = 0xFF };
    const char *txt = charOperations::skip_token(text, ' ');
    char *message;
    cnt = sscanf(txt, "%u %u %u", &tmp1, &tmp2, &tmp3);
    if (cnt == 1) {
        IoCommandManager::set3ParamForIoCmd((uint8_t) tmp1, fillerParam, fillerParam);
    } else if (cnt == 3) {
        IoCommandManager::set3ParamForIoCmd((uint8_t) tmp1, (uint8_t) tmp2, (uint8_t) tmp3);
    } else {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    }
    message = sendIoCmd2Peer((uint16_t) tmp1, ModemInterpreter::OutputRequest, IoCommandManager::noCMD);
    return message;
}


char* PlcTerminalCommandConnector::sendGlobalTimeout(char *text) {
    unsigned int id, delay, toAmount, coAmount;
    int32_t cnt;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u %u %u %u", &id, &delay, &toAmount, &coAmount);
    if ((cnt < 4) && (id > 20) && (delay > 1000) && (toAmount > 20)
            && (coAmount > 10)) {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    }
    // saves values in temporary memory; use RequestArgData as container
    uint8_t *pWert = RequestArgData;
    uint8Operations::insertUint16intoUint8Array(pWert, (uint16_t) id);
    pWert += 2;
    uint8Operations::insertUint16intoUint8Array(pWert, (uint16_t) delay);
    pWert += 2;
    uint8Operations::insertUint16intoUint8Array(pWert, (uint16_t) toAmount);
    pWert += 2;
    uint8Operations::insertUint16intoUint8Array(pWert, (uint16_t) coAmount);

    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->clearTimeout(ProgramConfig::simpleShortTOutID);
    ITimeOutWatcher::timeOutInfo *pToi = pW->findTimeOutWithID(
        ProgramConfig::simpleShortTOutID);
    if (pToi == NULL) {
        ITimeOutWatcher::timeOutInfo toi;
        ITimeOutWatcher::createDefaultTimeOut(&toi,
                                              ProgramConfig::simpleShortTOutID, &doSendGlobalTimeout, NULL);
        toi.timeCounter = 2;            //  2 * 0.05s = 100ms
        toi.delay = 40;                 // 40 * 0.05s = 2000ms
        toi.timeOutCounter = 5;
        toi.timeOutAmount = 5;
        toi.callOutCounter = 0;
        toi.callOutAmount = 0;
        pW->registerTimeout(toi);
        return const_cast<char*>(TX::getText(TX::GlobalSendTimoutMsg));
    }
    return const_cast<char*>("");
}


// Timeout Routine for globalTimeout function: send n times the global timeout command into the PLC-System
void PlcTerminalCommandConnector::doSendGlobalTimeout() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    ITimeOutWatcher::timeOutInfo *pToi = pW->findTimeOutWithID(ProgramConfig::simpleShortTOutID);
    if (pToi) {
        // return values from temporary memory
        unsigned int id, delay, toAmount, coAmount;
        uint8_t *pWert = RequestArgData;
        id = uint8Operations::sum2BytesFromLSB(pWert);
        pWert += 2;
        delay = uint8Operations::sum2BytesFromLSB(pWert);
        pWert += 2;
        toAmount = uint8Operations::sum2BytesFromLSB(pWert);
        pWert += 2;
        coAmount = uint8Operations::sum2BytesFromLSB(pWert);

        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%cGlobalTimeout %2u %4u %2u %2u",
                ModemInterpreter::commandPrefix, ModemInterpreter::TimeoutsGlobalRequest, id, delay, toAmount,
                coAmount);
        ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                (const uint8_t*) Global2::OutBuff,
                (uint8_t) strlen(Global2::OutBuff), IRoutingManager::BCGlobalIP,
                false, TermContextFilter::filterRoutingInfo);

        // set own timeout
        if (id != ProgramConfig::simpleShortTOutID) {
            pToi = pW->findTimeOutWithID((uint8_t) id);
            if (pToi) {
                pToi->delay = (uint16_t) delay;
                pToi->timeOutAmount = (uint8_t) toAmount;
                pToi->callOutCounter = (uint8_t) coAmount;
            }
        }
    }
}


char* PlcTerminalCommandConnector::terminalStartCmd(char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    if (strstr(txt, TX::getText(TX::CmdAlive))) {
        return ForwardingFlow::terminalStartAlive(txt);
    }
    if (strstr(txt, TX::getText(TX::CmdIntegrationTest))) {
        // get delay and number of peers for the test
        txt = charOperations::skip_token(txt, ' ');
        txt = charOperations::skip_token(txt, ' ');

        IntegrationsTests *pTest = ProgramConfig::getIntegrationTestPtr();
        unsigned int testTime, numberOfPeer, testNo;
        int num = sscanf(txt, "%u %u %u", &testNo, &testTime, &numberOfPeer);
        if (num >= 1) {
            pTest->setTestParameter(IntegrationsTests::eNumberOfTest, (IntegrationsTests::eTestcases) testNo);
        } else {
            pTest->setTestParameter(IntegrationsTests::eNumberOfTest,
                                    IntegrationsTests::eTestcases::CaseTest030AliveRoundTrips);
        }

        if (num >= 2) {
            pTest->setTestParameter(IntegrationsTests::eTestingTime, testTime);
        }

        if (num >= 3) {
            if (numberOfPeer > 1000) {
                return const_cast<char*>("?");
            }
            pTest->setTestParameter(IntegrationsTests::eNumberOfPeers, numberOfPeer);
        }
        pTest->setTestEnabled(true);
        ProgramConfig::getAutostartFlow().restartTestSuite();
        pTest->test030AliveRoundTrips(IntegrationsTests::eAliveTestMode::start);
        return const_cast<char*>("");
    }
    return const_cast<char*>("?");
}

char* PlcTerminalCommandConnector::terminalStopCmd(char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    if (strstr(txt, TX::getText(TX::CmdAlive))) {
        return ForwardingFlow::callStopAliveRoundTrips();
    }
    if (strstr(txt, TX::getText(TX::CmdIntegrationTest))) {
        IntegrationsTests *pTest = ProgramConfig::getIntegrationTestPtr();
        if (pTest->isAliveTestRunning()) {
            pTest->testAliveStop();
            return const_cast<char *>("");
        } else {
            return const_cast<char *>(TX::getText(TX::IntegrationTestStopped));
        }
    }
    return const_cast<char *>("?");
}


}  // namespace AME_SRC
