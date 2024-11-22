/*
 * ModemController.cpp
 *
 *  Created on: 19.09.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "ModemController.h"

#include <cstring>

#include "G3PlcSystem/UDPFrame.h"
#include "BootLoader/ModemBootLoader.h"
#include "ModemInterpreter.h"
#include "../SystemBase/ProgramConfig.h"
#include "../StorageManagement/SimpleFlashEditor.h"
#include "ModemTransmitter.h"
#include "../SystemLogic/StorageAdministrator.h"
#include "G3PlcSystem/RequestFactory/RequestFactory.h"
#include "PlcCommandTypeSpecification.h"
#include "../SignalProcessing/I2C/I2cApp.h"

namespace AME_SRC {
// using namespace PLC_CommandTypeSpecification;
Node<PlcCommand> *ModemController::processCmdNodePtr = NULL;
Node<PlcCommand> *ModemController::lastNodePtr = NULL;
uint8_t ModemController::lastNodeNr = 0;
NetworkMember ModemController::networkMemberList[networkMemberMax + 1];
bool ModemController::notWaitingForConfirmation = (true);
bool ModemController::initFinish = (false);
bool ModemController::firmenwareLoaded = false;
bool ModemController::receiveTokenAktiv = true;
bool ModemController::connectedToCoordinator = false;
bool ModemController::requestRepeatMode = true;
uint8_t ModemController::nBGTransmitMode = eBGTransmitMode::bgIdle;
bool ModemController::remotePathReplyAwait = false;
bool ModemController::aliveCheckEnabled = false;
bool ModemController::supressDisplayMessage = false;
bool ModemController::checkMemberPending = false;
uint16_t ModemController::aliveCheckPresetLoops = 0;
uint16_t ModemController::aliveCheckActLoops = 0;
uint8_t ModemController::memberListDisplayIndex = 0xFF;
uint8_t ModemController::aktivNetMemberCount = 0;
TimeMeasurement::measureJob *ModemController::aliveTimeOutJobPtr = 0;
uint8_t ModemController::newMemberRequest = 0;
uint8_t ModemController::callOutCounter = 0;
int ModemController::aliveSendFailCount = 0;
int ModemController::aliveTransmitIndex = 0;
int ModemController::globalDestIP = 0;

ModemController::PANDescriptor ModemController::panDescriptorContainer[maxPANDescriptorSets];
uint8_t ModemController::foundDescriptorAmount;
ModemController::controllerStates ModemController::controllerState =
        ModemController::init;
IoPin *ModemController::cpxResetPinPtr = NULL;
uint16_t ModemController::panId = 0x0002;
ModemController::displayState ModemController::actualDisplayState = displayIdle;
uint8_t ModemController::nodeStoreArrayIdx = 0;
Node<PlcCommand> ModemController::nodeStorageArray[maxNodeStoreArrayIdx];
List<PlcCommand> ModemController::nodeCmdDefaultList = List<PlcCommand>();
TermContextFilter *ModemController::termFilterPtr_ = NULL;

BitMask<uint32_t> ModemController::confirmMask = BitMask<uint32_t>();
uint8_t ModemController::outgoingNsduContainer[kNsduContainerSize] = { 0 };

ModemController::ModemController(IoPin *resetPin, SerialDriver *termPtr, AssignmentFlow &assignmentFlow) :
        newMemberJoined(false), networkMemberCount(1), assignmentFlow_(assignmentFlow) {
    termFilterPtr_ = new TermContextFilter(termPtr);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    cpxResetPinPtr = resetPin;
    aliveTransmitIndex = 0;
    aliveSendFailCount = 0;
    lastJoinedMemberIndex = 0;
    networkMemberList[0].setNetworkIPAddress(COORDINATOR_ID);
    setMacShortAddress((uint16_t) ProgramConfig::getPlcBoardId());
    ModemController::resetDiscoverySearchResults();
    for (uint16_t i = 0; i < networkMemberMax; i++) {
        networkMemberList[i].setLink(NetworkMember::disconnected);
        // ToDo: 07.06.2023 DS check code if it is still needed
        // if (ProgramConfig::isCoordinator() && !simpleFlashEditor::isForceJoinMode() && i>0) {
        //      updateStorageClientInfoByIdx(i);
        // }
    }
    // Commands to test the force join
//  static const uint8_t debugMacAddress[8] = {0x10,0x01,0x00,0xFF,0xFE,0x00,0x00,0x01};
//  networkMemberList[1] = NetworkMember(debugMacAddress,1);
//  networkMemberList[1].setIsActive(true);
}

ModemController::~ModemController() = default;

void ModemController::setConfirmationPosition(uint8_t position) {
    if (position < 32) {
        confirmMask.set(position);
        setNotWaitingForConfirmation(true);
    }
}

void ModemController::incrementProcessStateIfConfirmed() {
    if (processCmdNodePtr) {
        if (controllerState != idle) {  // (processCmdNodePtr != lastNodePtr) &&
            int confirmPosition = processCmdNodePtr->getData().getConfirmIndex();
            if (confirmMask.isSet(confirmPosition)) {
                confirmMask.unSet(confirmPosition);
                if (processCmdNodePtr->getNext() != NULL) {
                    Node<PlcCommand> *nextNodePtr = processCmdNodePtr->getNext();
                    lastNodePtr = processCmdNodePtr;
                    processCmdNodePtr = nextNodePtr;
                    confirmPosition = processCmdNodePtr->getData().getConfirmIndex();
                    confirmMask.unSet(confirmPosition);
                } else {
                    lastNodePtr = processCmdNodePtr;
                    processCmdNodePtr = NULL;
                }
                confirmMask.unSet(confirmPosition);
            }
        }
    } else {
        switch (controllerState) {
        case init:
            initFinish = true;
            controllerState = idle;
            break;
        case getForceJoinInfoX:
            setProcessState(createGetForceJoinInfo(false).getHead());
            break;
        case getForceJoinInfoLast:
            if (StorageAdministrator::getSaveAgainRequest()) {
                StorageAdministrator::releaseSaveAgainRequest();
                setProcessState(createGetForceJoinInfo(true).getHead());
            } else {
                ModemInterpreter::setNextInterpretState(
                        ModemInterpreter::updateCIFlashStart);  // networkMemberList, store ClientInfo in DataFlash
                controllerState = idle;
            }
            break;
        case setForceJoinInfoX:
            setProcessState(createSetForceJoinInfo(next).getHead());
            break;
        case getPLCAttributes:
            setProcessState(createAttributeDump(false).getHead());
            break;
        case aliveNeighbourParaNext:
            setProcessState(createSetGroupsWithGivenTupels(false, 0, 0, 0, NULL).getHead());
            break;
        default:
            controllerState = idle;
        }
    }
}

void ModemController::triggerNewDeviceAuthentification(
    const PlcCommand &requestPtr, CHANNEL channelId) {
    uint8_t macAddressPtr[8];
    bool isEntryValid = true;
    memcpy(macAddressPtr, requestPtr.getParameter(), 8);
    uint16_t macShortAdr = uint8Operations::sum2BytesFromLSB(macAddressPtr + 6);
    if (macShortAdr == 0) {
        return;
    }
    uint16_t knownListIndex = findNetworkmemberPositionWithMacAddress(
        macAddressPtr);
    bool isTheAddressKnown = (knownListIndex != 0);
    if (isTheAddressKnown) {
        networkMemberList[knownListIndex].setLink(NetworkMember::pending);
        triggerSinglePlcRequestWithArgs(setClientInfo, 4, channelId,
                isEntryValid, knownListIndex, (uint32_t) macAddressPtr);
    } else {
        uint32_t memberIdx = getNextInactiveNetworkIdx();
        if (memberIdx > 0) {
            NetworkMember &pn = networkMemberList[memberIdx];
            pn.setLink(NetworkMember::pending);
            pn.setPendingTime(120);
          pn.setNetworkIPAddress((uint16_t) memberIdx);
          pn.fillMacAddress(macAddressPtr);
            triggerSinglePlcRequestWithArgs(setClientInfo, 4, channelId,
                    isEntryValid, memberIdx, (uint32_t) macAddressPtr);
            //  updateStorageClientInfoByIP(pn.getNetworkIPAddress());
            //  ModemInterpreter::modemCrierPtr->println(StorageAdministrator::getStatusInfo());
        }
    }
}

// Update peer information in non-volatile memory
void ModemController::updateStorageClientInfoByIP(uint16_t memberIP) {
    NetworkMember *pn = getNetMemberPtrByIP(memberIP);
    if (pn) {
        uint16_t memberIdx = findNetworkmemberPositionWithMacAddress(
            pn->getMacAddress());
        updateStorageClientInfoByIdx(memberIdx);
    }
}

void ModemController::updateStorageClientInfoByIdx(uint16_t memberIdx) {
    NetworkMember *pn = &networkMemberList[memberIdx];
    uint16_t memberIP = pn->getNetworkIPAddress();
    bool entryInvalid = memberIP == 0xFF;
    StorageAdministrator::storePeerConnectionDetails(pn->getMacAddress(),
            memberIP, memberIdx, pn->getOutgoingRoute(), pn->getLink(),
            entryInvalid);
    char *info = StorageAdministrator::getStatusInfo();
    if (*info) {
        termFilterPtr_->println(info);
    }
}

PlcCommand* ModemController::executeProcessStateAndGetAnswer() {
    static const int timeOut = 6000;
    static const int callOut = 4;
    static uint8_t callOutCounter2 = 0;
    static int counter = 0;
    if (controllerState != idle) {
        uint8_t nr = processCmdNodePtr->getData().getCmdNr();
        if (lastNodePtr != processCmdNodePtr || lastNodeNr != nr) {
            if (counter++ > 50) {
                setNotWaitingForConfirmation(false);
                lastNodePtr = processCmdNodePtr;
                lastNodeNr = nr;
                counter = 0;
                callOutCounter2 = 0;
                return &processCmdNodePtr->getData();
            }
        } else if (isRequestRepeatMode()) {
            if (counter++ > timeOut) {
                if (callOutCounter2++ >= callOut) {
                    controllerState = idle;
                    return NULL;
                }
                lastNodePtr = NULL;
                notWaitingForConfirmation = true;
                counter = 0;
            }
            return NULL;
        }
    }
    return NULL;
}

int ModemController::findNextAktivMemberIndexFromPosition(int position) {
    for (int i = 0; i <= networkMemberMax; i++) {
        int continuingListIndex = (i + position) % (networkMemberMax + 1);
        NetworkMember *selectedMember = getNetMemberPtrByIndex(
                (uint16_t) continuingListIndex);
        if (selectedMember->getLink() == NetworkMember::connected) {
            return continuingListIndex;
        }
    }
    return -1;
}

// Function for restoring network participants after a reset on coordinator side (Force-Join)
void ModemController::restoreMemberList() {
    NetworkMember::linkStatus lnkState;
    IKeyContainerStorage *keyValueStore = StorageAdministrator::getStorageInterfacePtr();
    IKeyContainerStorage::skey rml_key;
    IKeyContainerStorage::container rml_con;
    rml_key.extended_key.key = StorageAdministrator::clientInfoTable;
    aktivNetMemberCount = 0;
    for (int i = 1; i <= networkMemberMax; i++) {
        rml_key.extended_key.index = (uint16_t) i;
        keyValueStore->loadContainerWithKey(&rml_key, &rml_con);
        if (rml_con.data && !rml_key.extended_key.flag.bit.invalid) {
            networkMemberList[i].setPendingTime(120);
          networkMemberList[i].setNetworkIPAddress(
              uint8Operations::sum2BytesFromLSB(
                  &rml_con.data[StorageAdministrator::ciOfsNwAdr]));
          networkMemberList[i].fillMacAddress(
              &rml_con.data[StorageAdministrator::ciOfsMacAdr]);
            networkMemberList[i].setOutgoingRoute(
                    (uint8_t) rml_con.data[StorageAdministrator::ciOfsOutgoingRoute]);
            lnkState =
                    (NetworkMember::linkStatus) rml_con.data[StorageAdministrator::ciOfsLinkStatus];
            if (lnkState == NetworkMember::connected || lnkState == NetworkMember::notVerified) {
                aktivNetMemberCount++;
                lnkState = NetworkMember::notVerified;
            } else {
                lnkState = NetworkMember::disconnected;
            }
        } else {
            lnkState = NetworkMember::disconnected;
        }
        if (lnkState == NetworkMember::disconnected) {
            if (networkMemberList[i].getLink() != NetworkMember::disconnected) {
                disableNetMemberAtIndex((uint16_t) i, false, true);
            }
        } else {
            networkMemberList[i].setLink(lnkState);
        }
    }
}

uint16_t ModemController::getNumPeersConnected(bool checkOutgoingRoute) {
    uint16_t result = 0;
    for (int i = 1; i <= networkMemberMax; i++) {
        NetworkMember &pMember = networkMemberList[i];
        if (pMember.getLink() != NetworkMember::disconnected) {
            if (checkOutgoingRoute) {
                if (pMember.getOutgoingRoute() == NetworkMember::verified) {
                    result++;
                }
            } else {
                result++;
            }
        }
    }
    return result;
}


uint16_t ModemController::getNumPeersAliveAssigned() {
  uint16_t result = 0;
  for (int i=1; i <= networkMemberMax; i++) {
      NetworkMember &pMember = networkMemberList[i];
      if (pMember.getLink() != NetworkMember::disconnected) {
          if (pMember.getAliveMember()) {
             result++;
          }
      }
  }
  return result;
}


void ModemController::setAliveMemberByIdx(uint16_t index, bool set) {
    NetworkMember *pn = getNetMemberPtrByIndex(index);
    if (pn) {
        pn->setAliveMember(set);
    }
}

void ModemController::setMemberErrorByIdx(uint16_t index, bool set) {
    NetworkMember *pn = getNetMemberPtrByIndex(index);
    if (pn) {
        pn->setAliveError(set);
    }
}


uint16_t ModemController::getNumPeersPending() {
    uint16_t result = 0;
    for (int i = 1; i <= networkMemberMax; i++) {
        NetworkMember &pMember = networkMemberList[i];
        if (pMember.getLink() == NetworkMember::pending) {
            result++;
        }
    }
    return result;
}

char* ModemController::getListPeersPending(char *output, uint32_t maxLen) {
    char temp[12];
    for (int i = 1; i <= networkMemberMax; i++) {
        NetworkMember &pMember = networkMemberList[i];
        if (pMember.getLink() == NetworkMember::pending) {
            if (strlen(output) < maxLen - 5) {
                if (*output) {
                    strncat(output, ", ", maxLen);
                }
                snprintf(temp, sizeof(temp), "%3d", i);
                strncat(output, temp, maxLen);
            }
        }
    }
    return output;
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void ModemController::triggerSinglePlcRequestWithArgs(
        plcRequestNames requestName, int count, ...) {
    static Node<PlcCommand> requestNode = Node<PlcCommand>();
    static PlcCommand plcCmd;
    va_list args;
    va_start(args, count);
    plcCmd = PlcCommand(RequestFactory::createRequest(requestName, args));
    va_end(args);
    requestNode.setDataReff(plcCmd);
    requestNode.setNextNull();
    setNotWaitingForConfirmation(true);
    controllerState = singleCmd;
    confirmMask.unSet(plcCmd.getConfirmIndex());
    processCmdNodePtr = &requestNode;
    lastNodePtr = NULL;
}

void ModemController::triggerSinglePlcRequestWithArgs(PlcCommand &plcCmd) {
    static Node<PlcCommand> requestNode = Node<PlcCommand>();

    requestNode.setDataReff(plcCmd);
    requestNode.setNextNull();
    setNotWaitingForConfirmation(true);
    controllerState = singleCmd;
    confirmMask.unSet(plcCmd.getConfirmIndex());
    processCmdNodePtr = &requestNode;
    lastNodePtr = NULL;
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void ModemController::displayNetworkList() {
    char dump[128];
    if (memberListDisplayIndex <= networkMemberMax) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "[%02d] ", memberListDisplayIndex);
        termFilterPtr_->print(Global2::OutBuff);
        NetworkMember displayNetMember = networkMemberList[memberListDisplayIndex];
        if (displayNetMember.getNetworkIPAddress() != 0Xff) {
            if (memberListDisplayIndex == 0) {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                        TX::getText(TX::cTxtRefMemberToString),
                        displayNetMember.toString(dump, sizeof(dump)));
                termFilterPtr_->println(Global2::OutBuff);
            } else {
                termFilterPtr_->println(displayNetMember.toString(dump, sizeof(dump)));
            }
        } else {
            termFilterPtr_->println(": %");
        }
        if (ProgramConfig::isCoordinator()) {
            memberListDisplayIndex++;
        } else {
            actualDisplayState = displayIdle;
        }
    } else {
        termFilterPtr_->println(TX::cTxtEndOfMemberList);
        ModemInterpreter::setInterpretState(ModemInterpreter::idle);
        actualDisplayState = displayIdle;
    }
}

// Find an entry with a short MAC address in the network list
NetworkMember* ModemController::findMemberWithShortID(uint16_t shortID) {
    NetworkMember *targetMemberPtr = NULL;
    for (uint32_t i = 0; i <= networkMemberMax; i++) {
        // Tobias
        // const uint8_t * targetMacAddress = networkMemberList[i].getMacAddress();
        // if(targetMacAddress[7] == shortID){
        if (networkMemberList[i].getMacShortAddress() == shortID) {
            targetMemberPtr = &networkMemberList[i];
            break;
        }
    }
    return targetMemberPtr;
}

uint8_t ModemController::findMemberIndexWithShortID(int shortID) {
    for (uint8_t i = 0; i < aktivNetMemberCount; i++) {
        const uint8_t *targetMacAddress = networkMemberList[i + 1].getMacAddress();
        if (targetMacAddress[7] == shortID) {
            return (uint8_t) (i + 1);
        }
    }
    return 0;
}

bool ModemController::activateMemberFromJoinIndication(
        const PlcCommand &joinIndication) {
    lastJoinedMemberIndex = findNetworkmemberPositionWithMacAddress(
        joinIndication.getParameter() + 1);
    if (lastJoinedMemberIndex > 0 && lastJoinedMemberIndex <= networkMemberMax) {
        NetworkMember &pn = networkMemberList[lastJoinedMemberIndex];
        uint8_t join_status = joinIndication.getParameter()[0];
        if (join_status == R_EAP_STATUS_EAP_PSK_FAILURE) {
            const uint32_t show =
                    (uint32_t) (TermContextFilter::filterForceJoinInfo) |
                    (uint32_t) (TermContextFilter::filterJoinTabs) |
                    (uint32_t) (TermContextFilter::filterForceJoinS);
            if (termFilterPtr_->isNotFiltered((TermContextFilter::filterBits) show)) {
                termFilterPtr_->println(TX::MemberRejected);
            }
            pn.setLink(NetworkMember::rejected);
            return false;
        }
        if (join_status != R_EAP_STATUS_SUCCESS) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::UntreatedJoinState), join_status);
            termFilterPtr_->println(Global2::OutBuff);
            return false;
        }
        newMemberJoined = pn.getLink() == NetworkMember::pending;
        pn.setLink(NetworkMember::connected);
        if (ProgramConfig::isCoordinator()) {
            updateStorageClientInfoByIdx(lastJoinedMemberIndex);
        }
        if (newMemberJoined) {
            aktivNetMemberCount++;
            if (++networkMemberCount > networkMemberMax) {
                networkMemberCount = networkMemberMax + 1;
            }
        }
        return true;
    } else {
        return false;
    }
}

// Test: add member to network list
// @param lastIP - last desired member
void ModemController::activateTestMembersUptoIP(uint16_t lastIP) {
    const uint16_t OfsIP = 100;     // IP 100... for new test member
    NetworkMember *pn;
    NetworkMember::linkStatus lnkState;
    for (uint32_t i = 1; i <= lastIP; i++) {
        int idx = getNetMemberIdxByIP((uint16_t) i);
        if (idx >= 0) {
            pn = &networkMemberList[idx];
            lnkState = (NetworkMember::linkStatus) pn->getLink();
            if (lnkState == NetworkMember::connected || lnkState == NetworkMember::notVerified) {
                continue;
            }
            pn->setLink(NetworkMember::connected);
            pn->setTestMember(true);
        } else {
            idx = aktivateNextFreeMemberWithIP((uint16_t) i, true);
            if (idx >= 0) {
                pn = &networkMemberList[idx];
              pn->setMacShortAddress((uint16_t) (OfsIP + i));
                pn->setTestMember(true);
            } else {
                break;
            }
        }
    }
}

void ModemController::deActivateAllMembers() {
    for (NetworkMember member : networkMemberList) {
        member.setLink(NetworkMember::disconnected);
    }
}

uint16_t ModemController::findNetworkmemberPositionWithMacAddress(
        const uint8_t *macAddress) {
    for (uint16_t i = 0; i <= networkMemberMax; i++) {
        if (networkMemberList[i].hasEqualMacAddress(macAddress)) {
            return i;
        }
    }
    return 0;
}

#pragma GCC diagnostic ignored "-Wstack-usage="
List<PlcCommand>& ModemController::createInitPlcReqListForChannel(CHANNEL channelID) {
    // using namespace PLC_CommandTypeSpecification;
    List<PlcCommand> &initList = nodeCmdDefaultList;
    CPX_MODE mode = ProgramConfig::isCoordinator() ? COORDINATOR : PEER;
    Node<PlcCommand > &initNode = nodeStorageArray[0];
    Node<PlcCommand > &setConfigNode = nodeStorageArray[1];
    Node<PlcCommand > &adpResetNode = nodeStorageArray[2];
    Node<PlcCommand > &setDeviceTypeNode = nodeStorageArray[3];
    Node<PlcCommand > &eapResetNode = nodeStorageArray[4];
    Node<PlcCommand > &setBroadCastNode1 = nodeStorageArray[5];
    Node<PlcCommand > &setBroadCastNode2 = nodeStorageArray[12];
    Node<PlcCommand > &eapPskRetryNode = nodeStorageArray[6];
    Node<PlcCommand > &setDefaultCoordNode = nodeStorageArray[7];
    Node<PlcCommand > &setGMKNode = nodeStorageArray[8];
    Node<PlcCommand > &networkStartNode = nodeStorageArray[9];
    Node<PlcCommand > &eapmStartNode = nodeStorageArray[10];
    Node<PlcCommand > &systemVersionNode = nodeStorageArray[11];
    initNode = Node<PlcCommand>(&RequestFactory::createRequest(channelInit, 2, channelID, mode));
    setConfigNode = Node<PlcCommand>(
                    &RequestFactory::createRequest(setConfig, 2, channelID,
                       (uint32_t) networkMemberList[(ProgramConfig::isCoordinator()
                           == false)].getMacAddress()));  // TODO(AME) make switchable: FCC & CENELEC A
    adpResetNode = Node<PlcCommand>(
            &RequestFactory::createRequest(adpReset, 2, channelID, mode));
    setDeviceTypeNode = Node<PlcCommand>(
            &RequestFactory::createUserSetRequest(DeviceTypeChannel0, 1, mode));

    eapResetNode = Node<PlcCommand>(
            &RequestFactory::createRequest(eapReset, 1, channelID));
    uint8_t multicastBroadcstAddress[] = {0x01, 0x80, 0xFF };
    setBroadCastNode1 = Node<PlcCommand>(
            &RequestFactory::createRequest(setAdpAttribute, 5, channelID,
                    adpGroupTable, 0, sizeof(multicastBroadcstAddress),
                    (uint32_t) multicastBroadcstAddress));
  multicastBroadcstAddress[2] = IRoutingManager::BCAliveIP;
    setBroadCastNode2 = Node<PlcCommand>(
            &RequestFactory::createRequest(setAdpAttribute, 5, channelID,
                    adpGroupTable, 1, sizeof(multicastBroadcstAddress),
                    (uint32_t) multicastBroadcstAddress));

    eapPskRetryNode = Node<PlcCommand>(
            &RequestFactory::createUserSetRequest(pskRetryNumChannel0, 1, 8));

    setDefaultCoordNode = Node<PlcCommand>(
            &RequestFactory::createUserSetRequest(DefaultCoordChannel0, 1,
                    true));

    setGMKNode = Node<PlcCommand>(
            &RequestFactory::createUserSetRequest(eapGMKChannel0, 0));
    networkStartNode = Node<PlcCommand>(
            &RequestFactory::createRequest(networkStart, 2, channelID, panId));
    eapmStartNode = Node<PlcCommand>(
            &RequestFactory::createRequest(eapmStart, 1, channelID));
    systemVersionNode = Node<PlcCommand>(
            &RequestFactory::createRequest(getSystemVersion, 1, channelID));

    initList.clear();
    initList.add(initNode);
    initList.add(setConfigNode);
    initList.add(adpResetNode);
    initList.add(setBroadCastNode1);
    initList.add(setBroadCastNode2);
    if (mode == COORDINATOR) {
        initList.add(eapResetNode);
        initList.add(eapPskRetryNode);
        initList.add(setDeviceTypeNode);
        initList.add(setGMKNode);
        initList.add(networkStartNode);
        initList.add(eapmStartNode);
        // initList = initList + createStartNetworkPlcReqLisForChannel(channelID);
        // createStartNetworkPlcReqLisForChannel(channelID);
    } else {
        initList.add(setDefaultCoordNode);
        initList.add(setDeviceTypeNode);
    }
    initList.add(systemVersionNode);
    controllerState = init;
    return initList;
}

#pragma GCC diagnostic ignored "-Wstack-usage="
List<PlcCommand>& ModemController::createStartNetworkPlcReqLisForChannel(
        CHANNEL channelID) {
    static List<PlcCommand> networkStartList = List<PlcCommand>();
    // List<PlcCommand>  & networkStartList = nodeCmdDefaultList;
    Node<PlcCommand > &setGMKNode = nodeStorageArray[8];
    Node<PlcCommand > &networkStartNode = nodeStorageArray[9];
    Node<PlcCommand > &eapmStartNode = nodeStorageArray[10];
    setGMKNode = Node<PlcCommand>(
            &RequestFactory::createUserSetRequest(eapGMKChannel0, 0));
    networkStartNode = Node<PlcCommand>(
            &RequestFactory::createRequest(networkStart, 2, channelID, panId));
    eapmStartNode = Node<PlcCommand>(
            &RequestFactory::createRequest(eapmStart, 1, channelID));

    networkStartList.add(setGMKNode);
    networkStartList.add(networkStartNode);
    networkStartList.add(eapmStartNode);

    return networkStartList;
}

//// List with nodes to load and save the force join information
// static List<PlcCommand>getSetForceJoinInfoList;
// static Node<PlcCommand>getSetRoutingTableNode;
// static Node<PlcCommand>getSetNeighbourTableNode;
// static Node<PlcCommand>getSetFrameCounterNode;

// Parameter start: true - reset index to 0 (1st element in a table)
#pragma GCC diagnostic ignored "-Wstack-usage="
List<PlcCommand>& ModemController::createGetForceJoinInfo(bool lStart) {
    CHANNEL id = channel0;
    enum eState {
        getInfo, getFC
    };
    static uint8_t getState = getInfo;
    static uint8_t getForceJoinIdx = 0;  // 0..5
    if (lStart) {
        getForceJoinIdx = 0;
        getState = getInfo;
    }
    Node<PlcCommand > &getSetRoutingTableNode = nodeStorageArray[0];
    Node<PlcCommand > &getSetNeighbourTableNode = nodeStorageArray[1];
    Node<PlcCommand > &getSetFrameCounterNode = nodeStorageArray[2];
    List<PlcCommand> &getSetForceJoinInfoList = nodeCmdDefaultList;
    getSetForceJoinInfoList.clear();

    switch (getState) {
    case getInfo:
        getSetRoutingTableNode = Node<PlcCommand>(
                &RequestFactory::createRequest(getAdpAttribute, 3, id,
                        adpRoutingTable, getForceJoinIdx));
        getSetNeighbourTableNode = Node<PlcCommand>(
                &RequestFactory::createRequest(getMacAttribute, 3, id,
                        macNeighbourTable, getForceJoinIdx));
        getSetForceJoinInfoList.add(getSetRoutingTableNode);
        getSetForceJoinInfoList.add(getSetNeighbourTableNode);
        controllerState = getForceJoinInfoX;
        if (ProgramConfig::isCoordinator()) {
            if (++getForceJoinIdx >= networkMemberMax) {
                getForceJoinIdx = 0;
                getState = getFC;
            }
        } else {  // Peer
            if (++getForceJoinIdx >= 5) {
                getForceJoinIdx = 0;
                getState = getFC;
            }
        }
        break;
    default:  // getFC
        if (!ProgramConfig::isCoordinator()) {
            // store network address if needed
            uint16_t nw_adr = networkMemberList[1].getNetworkIPAddress();
            if ((nw_adr != 0xFF) && (nw_adr > 0)) {
              SimpleFlashEditor::setOwnNetworkAddress(nw_adr);
            }
        }
        getSetFrameCounterNode = Node<PlcCommand>(
                &RequestFactory::createRequest(getMacAttribute, 3, id,
                        macFrameCounter, 0));
        getSetForceJoinInfoList.add(getSetFrameCounterNode);
        getState = getInfo;
        controllerState = getForceJoinInfoLast;
    }
    return getSetForceJoinInfoList;
}

// Parameter start: true - reset index to 0 (1st element in a table)
#pragma GCC diagnostic ignored "-Wstack-usage="
List<PlcCommand>& ModemController::createSetForceJoinInfo(
        enum createSetForceAttrMode nMode) {
    CHANNEL id = channel0;
    enum eState {
        setInfo, setFC, setCI
    };
    static uint8_t setState = setInfo;
    static uint8_t setForceJoinIdx = 0;     // 0..5
    static uint8_t setNodeIdx = 0;          // 0..2
    switch (nMode) {
    case fc_only:
        setState = setFC;
        break;
    case start:
        setForceJoinIdx = 0;
        setState = setInfo;
        // no break
    default:
        break;
    }
//  Node<PlcCommand > & getSetRoutingTableNode   = nodeStorageArray[0];
//  Node<PlcCommand > & getSetNeighbourTableNode = nodeStorageArray[1];
//  Node<PlcCommand > & getSetFrameCounterNode   = nodeStorageArray[2];
    List<PlcCommand> &getSetForceJoinInfoList = nodeCmdDefaultList;
    getSetForceJoinInfoList.clear();
    // getRoutingTableNode.getData().setParam(XYZ) = adpRouteIdx;

    static IKeyContainerStorage::container attributeContainer;
    IKeyContainerStorage *keyValueStore =
            StorageAdministrator::getStorageInterfacePtr();
    IKeyContainerStorage::skey temp_key;

    switch (setState) {
    case setInfo:
        temp_key.extended_key.key = StorageAdministrator::adpRouteTable;
        temp_key.extended_key.index = setForceJoinIdx;
        keyValueStore->loadContainerWithKey(&temp_key, &attributeContainer);
        if (attributeContainer.data) {
            uint8_t adpBuf[10];
            memcpy(adpBuf, attributeContainer.data, 10);  // attributeContainer.length=10
            uint8_t *pValidTime = &adpBuf[8];
            uint8Operations::insertUint16intoUint8Array(pValidTime, 360);
            if (adpBuf[6]) {    // Hops!=0
                Node<PlcCommand> &SetNodeRef0 = nodeStorageArray[setNodeIdx];
                SetNodeRef0 = Node<PlcCommand>(
                        &RequestFactory::createRequest(setAdpAttribute, 4, id,
                                adpRoutingTable, setForceJoinIdx,
                                attributeContainer.length, adpBuf));
                getSetForceJoinInfoList.add(SetNodeRef0);
            }
            if (++setNodeIdx > 2)
                setNodeIdx = 0;
        }

        temp_key.extended_key.key = StorageAdministrator::macNeighbourTable;
//      temp_key.extended_key.index = setForceJoinIdx;
        keyValueStore->loadContainerWithKey(&temp_key, &attributeContainer);
        if (attributeContainer.data) {
            Node<PlcCommand> &SetNodeRef1 = nodeStorageArray[setNodeIdx];
            SetNodeRef1 = Node<PlcCommand>(
                    &RequestFactory::createRequest(setMacAttribute, 4, id,
                            macNeighbourTable, setForceJoinIdx,
                            attributeContainer.length,
                            attributeContainer.data));
            getSetForceJoinInfoList.add(SetNodeRef1);
        }
        if (ProgramConfig::isCoordinator()) {
            if (++setForceJoinIdx >= networkMemberMax) {
                setForceJoinIdx = 1;    // 1st peer
                setState = setCI;
            }
        } else {  // peer
            if (++setForceJoinIdx >= 5) {
                setForceJoinIdx = 0;
                setState = setFC;
            }
        }
        controllerState = setForceJoinInfoX;
        break;
    case setFC:
        temp_key.extended_key.key = StorageAdministrator::frameCounter;
        temp_key.extended_key.index = 0;
        keyValueStore->loadContainerWithKey(&temp_key, &attributeContainer);
        if (attributeContainer.data) {
            // Use a higher value prophylactically for the frame counter
            // take Endian format into account
            uint32_t tmpFC_LE, tmpFC_BE;
            tmpFC_LE = uint8Operations::sum4BytesFromLSB(
                    attributeContainer.data);
            tmpFC_LE += 5;
            uint8Operations::insertUint32intoUint8Array(reinterpret_cast<uint8_t*>(&tmpFC_BE),
                    tmpFC_LE);
            Node<PlcCommand> &SetNodeRef2 = nodeStorageArray[setNodeIdx];
            SetNodeRef2 = Node<PlcCommand>(
                    &RequestFactory::createRequest(setMacAttribute, 4, id,
                            macFrameCounter, 0, attributeContainer.length,
                            &tmpFC_BE));
            getSetForceJoinInfoList.add(SetNodeRef2);
        }
        setState = setInfo;
        controllerState = setForceJoinInfoLast;
        break;
    case setCI:     // Coordinator only: setClientInfo
        temp_key.extended_key.key = StorageAdministrator::clientInfoTable;
        temp_key.extended_key.index = setForceJoinIdx;
        keyValueStore->loadContainerWithKey(&temp_key, &attributeContainer);
        if (attributeContainer.data) {
            controllerState = setForceJoinInfoX;
            Node<PlcCommand> &SetNodeRef3 = nodeStorageArray[setNodeIdx];
            NetworkMember &pn = networkMemberList[setForceJoinIdx];
            //  pn.setLink(NetworkMember::notVerified);     // refer to restoreMemberList()
            //  pn.setPendingTime(120);                     // refer to restoreMemberList()
          pn.setNetworkIPAddress(
              uint8Operations::sum2BytesFromLSB(
                  &attributeContainer.data[StorageAdministrator::ciOfsNwAdr]));
          pn.fillMacAddress(
              &attributeContainer.data[StorageAdministrator::ciOfsMacAdr]);
            // refer to restoreMemberList()
            // pn.setOutgoingRoute((uint8_t) attributeContainer.data[StorageAdministrator::ciOfsOutgoingRoute]);
            // triggerSinglePlcRequestWithArgs(setClientInfo,4,id,true,pn.getNetworkIPAddress(),(uint32_t)attributeContainer.data);
            // setNotWaitingForConfirmation(false);
            SetNodeRef3 = Node<PlcCommand>(
                    &RequestFactory::createRequest(setClientInfo, 4, id, true,
                                                   pn.getNetworkIPAddress(), attributeContainer.data));
            getSetForceJoinInfoList.add(SetNodeRef3);
        }
        if (++setForceJoinIdx >= networkMemberMax) {
            setForceJoinIdx = 0;
            setState = setFC;   // set frame counter also at coordinator
        }
        controllerState = setForceJoinInfoX;
        break;
    }
    if (++setNodeIdx > 2)
        setNodeIdx = 0;
    return getSetForceJoinInfoList;
}

// Parameter start: true - reset index to 0 (1st element in the attribute list)
#pragma GCC diagnostic ignored "-Wstack-usage="
List<PlcCommand>& ModemController::createAttributeDump(bool lStart) {
    CHANNEL id = channel0;
    static uint16_t getAttributeIdx = 0;
    static bool lGetAdpAttributes = true;
    if (lStart) {
        getAttributeIdx = 0;
        lGetAdpAttributes = true;
    }
//  Node<PlcCommand>& getAttributeTableNode = nodeStorageArray[0];
    List<PlcCommand> &getAttributeInfoList = nodeCmdDefaultList;
//  getAttributeTableNode.setNextNull();
    getAttributeInfoList.clear();

    for (int i = 0; i <= 1; i++) {
        if (lGetAdpAttributes) {
            uint16_t adpAttributeAmount = sizeof(adpFetchAttributes)
                    / sizeof(uint8_t);
            if (getAttributeIdx < adpAttributeAmount) {
                nodeStorageArray[i] = Node<PlcCommand>(
                        &RequestFactory::createRequest(getAdpAttribute, 3, id,
                                adpFetchAttributes[getAttributeIdx], 0));
                getAttributeInfoList.add(nodeStorageArray[i]);
                controllerState = getPLCAttributes;
                if (++getAttributeIdx == adpAttributeAmount) {
                    // controllerState = getPLCAttrLast;
                    lGetAdpAttributes = false;
                    getAttributeIdx = 0;
                    // break;
                }
            }
        } else {
            uint16_t umacAttributeAmount = sizeof(umacFetchAttributes)
                    / sizeof(uint16_t);
            if (getAttributeIdx < umacAttributeAmount) {
                nodeStorageArray[i] = Node<PlcCommand>(
                        &RequestFactory::createRequest(getMacAttribute, 3, id,
                                umacFetchAttributes[getAttributeIdx], 0));
                getAttributeInfoList.add(nodeStorageArray[i]);
                controllerState = getPLCAttributes;
                if (++getAttributeIdx == umacAttributeAmount) {
                    controllerState = getPLCAttrLast;
                    lGetAdpAttributes = true;
                    getAttributeIdx = 0;
                    break;
                }
            }
        }
    }

    return getAttributeInfoList;
}

/***
 * Short: fill nodeStorageArray entry with a message to coordinator
 * @param index         Index in the nodeStorageArray
 * @param prefix        added abbreviation (character) for a SearchRequest message to identify the type of answer
 * @param *data         pointer to data which are added. Pointer may be NULL. Data can also be in hex format
 * @param dataLength    Number of bytes in *data stream
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void ModemController::storeDataAtNodeIndex(uint32_t index, char prefix,
        const uint8_t *data, uint8_t dataLength) {
    enum {
        noBroadcast = false, dataSendCmdOffset = 2
    };
    const uint8_t netIP = 0;    // Coordinator
    uint16_t nsduLength, ipAdr;
    uint32_t dLen = strlen(ModemInterpreter::commandPrefix) + 1 + 1 + 5;  // 1:SearchRequest 1:prefix 5:ipAdr
    ipAdr = networkMemberList[1].getNetworkIPAddress();
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%c%c%04d ", ModemInterpreter::commandPrefix,
            ModemInterpreter::SearchRequest, prefix, ipAdr);
    if (data != NULL && dataLength != 0) {
        memcpy(&Global2::OutBuff[dLen], data, dataLength);
        dLen += dataLength;
    }
    nsduLength = createIPv6udpFrame(outgoingNsduContainer + dataSendCmdOffset,
            (const uint8_t*) Global2::OutBuff, (uint8_t) dLen, netIP,
            noBroadcast);
    nodeStorageArray[index] = Node<PlcCommand>(
            &RequestFactory::createRequest(
                    PLC_CommandTypeSpecification::sendData, 5, channel0,
                    (uint32_t) outgoingNsduContainer, nsduLength, noBroadcast,
                    false));
}

/***
 * This Method creates plc Requests which parameterize the cpx Modem
 * for certain Multicast Group Table. This is necessary in order
 * to receive multicast transmissions.
 * @return
 */
/*** DS, 22.11.2023: Obsolet
 List<PlcCommand>& ModemController::createSetGroupsWithGivenIPs(uint8_t startIndex,uint8_t count,...){
 va_list groupIPs;
 uint16_t ip;
 List<PlcCommand>  & setNeighbourGroupList = nodeCmdDefaultList;
 CHANNEL id = channel0;
 enum {prefabLen=3};
 uint8_t multicastPrefab[] ={0x01,0x80,0xFF};

 va_start(groupIPs,count);
 uint32_t listIdx = 0;
 for ( int groupIdx = 0; groupIdx < count; groupIdx++ )
 {
 ip = (uint16_t) va_arg ( groupIPs, int );
 ip |= (uint16_t)0x8000;
 uint8Operations::insertUint16intoUint8Array(&multicastPrefab[1], ip);
 nodeStorageArray[listIdx] = Node<PlcCommand>(&RequestFactory::createRequest(setAdpAttribute,5,id,adpGroupTable,startIndex+groupIdx,prefabLen,(uint32_t)multicastPrefab));
 setNeighbourGroupList.add(nodeStorageArray[listIdx++]);
 nodeStorageArray[listIdx] = Node<PlcCommand>(&RequestFactory::createRequest(getSystemVersion,1,channel0));
 setNeighbourGroupList.add(nodeStorageArray[listIdx++]);
 }
 va_end(groupIPs);
 controllerState = aliveNeighbourParam;
 return setNeighbourGroupList;
 }
 ***/

/***
 * This Method creates plc Requests which parameterize the cpx Modem
 * for certain Multicast Group Table. This is necessary in order
 * to receive multicast transmissions.
 * @return
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
List<PlcCommand>& ModemController::createSetGroupsWithGivenTupels(bool lStart,
                                                                  uint8_t startIndexModem, uint8_t startIndexAliveMap, uint8_t count,
                                                                  const IRoutingManager::MemberIpTupel *tupelArray) {
    static const IRoutingManager::MemberIpTupel *pTupel = NULL;
    static uint8_t tupelAmount = 0;
    static uint8_t currentIdx = 0;  // Offset in TupelArray
    static uint8_t currentOfs = 0;  // Offset in Modem
    if (lStart) {
        pTupel = tupelArray;
        tupelAmount = count;
        currentIdx = startIndexAliveMap;
        currentOfs = startIndexModem;
    }
    IRoutingManager::MemberIpTupel tupel;
    List<PlcCommand> &setNeighbourGroupList = nodeCmdDefaultList;
    CHANNEL id = channel0;
    enum {
        prefabLen = 3
    };
    uint8_t multicastPrefab[] = { 0x01, 0x80, 0xFF };
    controllerStates presetState = aliveNeighbourParam;
    uint32_t groupIdx, listIdx = 0;
    for (groupIdx = currentIdx; groupIdx < tupelAmount; groupIdx++) {
        tupel = pTupel[groupIdx];
        tupel.ip |= 0x8000;
        uint8Operations::insertUint16intoUint8Array(&multicastPrefab[1],
                tupel.ip);
        nodeStorageArray[listIdx] = Node<PlcCommand>(
                &RequestFactory::createRequest(setAdpAttribute, 5, id,
                        adpGroupTable, currentOfs, prefabLen,
                        (uint32_t) multicastPrefab));
        setNeighbourGroupList.add(nodeStorageArray[listIdx++]);
//      // Version query incorporated so that all groups can be set successfully.
//      nodeStorageArray[listIdx] = Node<PlcCommand>(&RequestFactory::createRequest(getSystemVersion,1,id));
//      setNeighbourGroupList.add(nodeStorageArray[listIdx++]);
        if (listIdx >= maxNodeStoreArrayIdx) {
            presetState = aliveNeighbourParaNext;
            break;
        }
        currentIdx++;
        currentOfs++;
    }
    setNeighbourGroupList.setHead(nodeStorageArray[0]);
    controllerState = presetState;
    return setNeighbourGroupList;
}

// Create UDP Frame with IPv6Header
#pragma GCC diagnostic ignored "-Wstack-usage="
uint16_t ModemController::createIPv6udpFrame(uint8_t *nsdu, const uint8_t *msg,
        uint8_t msg_len, uint16_t dest_net_ip, bool broadCast) {
    uint8_t tmpNsduContainer[kNsduContainerSize];
    uint8_t destBroadcastAddress[16] = {0xFF, 0x02, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x0, 0x00, 0x00, 0x0, 0xFF, 0xFF };
    uint8_t destUnicastAddress[16] = {0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0xFF, 0xFF, 0x0, 0xFF, 0xFE, 0x0, 0xFF, 0xFF };
    uint8_t *destAddressPtr;
    if (broadCast) {
      destAddressPtr = destBroadcastAddress;
        uint8Operations::insertUint16intoUint8Array(destAddressPtr + 14,
                                                    dest_net_ip);
    } else {
      destAddressPtr = destUnicastAddress;
        uint8Operations::insertUint16intoUint8Array(destAddressPtr + 8, panId);
        uint8Operations::insertUint16intoUint8Array(destAddressPtr + 14,
                                                    dest_net_ip);
    }

    uint16_t payloadLength = (uint16_t) (sizeof(UDPFrame::UDPformat) + msg_len
            + 1);
    uint8_t sourceAddress[16] = {0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0xFF, 0xFF, 0x0, 0xFF, 0xFE, 0x0, 0xFF, 0xFF };
    uint8Operations::insertUint16intoUint8Array(sourceAddress + 8, panId);
    uint16_t sourceIPshortAddress = ModemController::getNetworkIPAddress();
    uint8Operations::insertUint16intoUint8Array(sourceAddress + 14,
                                                sourceIPshortAddress);
    UDPFrame::IPv6Header myIPv6Header =
            {0x6, 0x0, 0x0, payloadLength, 0x11/*Next Header*/,
             0x0F/* Hop Limit*/, sourceAddress, destAddressPtr };
    UDPFrame::UDPformat myUDPFormat = { 0xABCD, 0xF0BF, payloadLength, 0x0000 };
    UDPFrame myFrame(myIPv6Header, myUDPFormat, (uint8_t*) msg);
    myFrame.calculateUDPChecksum();
    uint16_t nsduLength =
            (uint16_t) (payloadLength + UDPFrame::ipv6HeaderLength);
    memcpy(nsdu, myFrame.toArray(tmpNsduContainer), nsduLength);
    return nsduLength;
}

#if ALIVE_CHECK_LEVEL >= 2
List<PlcCommand>& ModemController::createAliveRequest(CHANNEL channelID) {
    CPX_MODE mode = ProgramConfig::isCoordinator() ? COORDINATOR : PEER;
    static uint8_t nsdu1[kNsduContainerSize] = {};
    #if ALIVE_CHECK_WITH_ACK >= 1
    static char nsdu2[kNsduContainerSize] = {};
    #endif

    char* pAlive = ModemInterpreter::getAliveChainMessagePtr();
    #if ALIVE_CHECK_WITH_ACK >= 1
    char* pAck   = ModemInterpreter::getAliveChainAckMsgPtr();
    #endif
    uint16_t nsdu_len1 = createIPv6udpFrame(nsdu1, (const uint8_t *)pAlive, strlen(pAlive),
                         ModemController::getAliveNextGroupe(), false);
    #if ALIVE_CHECK_WITH_ACK >= 1
    uint16_t nsdu_len2 = createIPv6udpFrame(nsdu2, pAck,   strlen(pAck), ModemController::getAlivePrevMember());
    #endif

    static List<PlcCommand>aliveRequestList = List<PlcCommand>();
    static Node<PlcCommand>forwardAlivetNode =
            Node<PlcCommand>(&RequestFactory::createRequest(sendData, 4, channelID, nsdu1, nsdu_len1, false));
    #if ALIVE_CHECK_WITH_ACK >= 1
    static Node<PlcCommand> ackAliveNode =
           Node<PlcCommand>(&RequestFactory::createRequest(sendData, 4, channelID, nsdu2, nsdu_len2, false));
    #endif

    aliveRequestList.add(forwardAlivetNode);
    #if ALIVE_CHECK_WITH_ACK >= 1
    aliveRequestList.add(ackAliveNode);
    #endif
    controllerState = aliveRequest;
    return aliveRequestList;
}
#endif

// PlcCommand* ModemController::getPlcCommandAndSetTargetLayerAndMessage(plcExecuteData* executeData ) {
//  PlcCommand *plcRequest = 0;
//  if(executeData) {
//      plcRequest = RequestFactory::createRequest(
//                      executeData->requestName,
//                      executeData->argAmount,
//                      executeData->commandCreationArgs[0],
//                      executeData->commandCreationArgs[1],
//                      executeData->commandCreationArgs[2],
//                      executeData->commandCreationArgs[3]
//                  );
//      plcRequest->setMessagePtr(executeData->message);
//  }
//  return plcRequest;
//  return NULL;
// }

struct plcCommandContainer {
    PlcCommand *request;
    const plcCommandContainer *nextPtr;
};

void ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(
        char *message, uint16_t netIP, bool routingMode) {
    triggerSendDataWithLengthToNetIPAndRoutingMode(reinterpret_cast<uint8_t*>(message),
            (uint8_t) strlen(message), netIP, routingMode);
}

void ModemController::triggerSendDataWithLengthToNetIPAndRoutingMode(
        const uint8_t *message, uint8_t length, uint16_t netIP,
        bool routingMode) {
    enum {
        noBroadcast = false, dataSendCmdOffset = 2
    };

    uint16_t nsduLength = createIPv6udpFrame(
            outgoingNsduContainer + dataSendCmdOffset, message, length, netIP,
            static_cast<bool>(noBroadcast));
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::sendData, 4, channel0,
            (uint32_t) outgoingNsduContainer, nsduLength, routingMode, false);
}

void ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
        const uint8_t *message, uint8_t length, uint16_t groupeIP,
        bool isImportant, uint32_t ShowMask) {
    enum {
        broadcast = true, dataSendCmdOffset = 2
    };

    if (ShowMask && termFilterPtr_->isNotFiltered((TermContextFilter::filterBits) ShowMask)) {
        char tempBuf[12];
        termFilterPtr_->putchar(0x0d);
        termFilterPtr_->putchar(0x54);    // T
        termFilterPtr_->putchar(0x78);    // x
        termFilterPtr_->putchar(0x3a);    // :
        termFilterPtr_->print(reinterpret_cast<const char*>(message));
        snprintf(tempBuf, sizeof(tempBuf), " -> %d", groupeIP);
        termFilterPtr_->println(tempBuf);
    }

    uint16_t nsduLength = createIPv6udpFrame(
            outgoingNsduContainer + dataSendCmdOffset, message, length,
            groupeIP, broadcast);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::sendData, 5, channel0,
            (uint32_t) outgoingNsduContainer, nsduLength, false, isImportant);
}

// Handling of the PAN Descriptor Discovery Search Results
#define PAN_DESCRIPTOR_HANDLING
#ifdef PAN_DESCRIPTOR_HANDLING

/***
 * Short: Transform a Sequenz into a PANDescriptor and Store it at a given Index
 * @param sequenzPtr		PANDescriptor start Pointer
 * @param containerIndex	Index in the Container, the Descriptor is transferred to
 */
void ModemController::transformSequenzToPANDescriptorAndStoreAtContainerIndex(
        const uint8_t *sequenzPtr, uint8_t containerIndex) {
    enum discriptorOffsetNames {
        panID = 2, adress = 4, quality = 6, routingCost = 8
    };
    const enum discriptorOffsetNames discriptorOffsets[] = { panID, adress,
            quality, routingCost };

    for (uint8_t offsetIndex = 0;
            offsetIndex < (sizeof(discriptorOffsets) / sizeof(int));
            offsetIndex++) {
        const uint8_t *itemValuePtr = sequenzPtr
                + discriptorOffsets[offsetIndex];
        uint16_t itemValue = uint8Operations::sum2BytesFromLSB(itemValuePtr);
        panDescriptorContainer[containerIndex].discriptorItem[offsetIndex] =
                itemValue;
    }
}

/***
 * Short: transfer the discovery confirmation into the "panDescriptorContainer"
 * Description: Stores the first 5 PandDescriptor Entrys of the discovery confirmation from the cpx-Modem
 * into an Array called "panDescriptorContainer".
 * @param confirmation PLC-Command discovery confirmation as result of an discovery request to the CPX-Modem.
 */
void ModemController::storePANDiscriptorContainerFromDiscoveryConfirmation(
        const PlcCommand &confirmation) {
    uint8_t PanDescriptorLength = 7;
    foundDescriptorAmount = confirmation.getParameter()[1];
    if (foundDescriptorAmount > maxPANDescriptorSets) {
        foundDescriptorAmount = maxPANDescriptorSets;
    }
    for (uint8_t descriptorIndex = 0; descriptorIndex < foundDescriptorAmount; descriptorIndex++) {
        uint8_t currentOffset =
                (uint8_t) (descriptorIndex * PanDescriptorLength);
        const uint8_t *sequenzPtr = confirmation.getParameter() + currentOffset;

        transformSequenzToPANDescriptorAndStoreAtContainerIndex(sequenzPtr,
                descriptorIndex);
    }
}

/**
 * Short: search the best PanDescriptor from panDescriptorContainer
 * Description: In this method, the panDescriptorContainer is sequential analyzed. With
 * each row, it is checked if the current row link value is the best value in comparison to all
 * previous link values.
 * @return PANDescriptor Pointer with best combination of linkQuality and Routing Cost
 */
ModemController::PANDescriptor* ModemController::getbestPanDescriptorFromDescriptorContainer() {
    double bestLinkValue = 0;
    double currentLinkValue = 0;
    PANDescriptor *resultDescriptorPtr = 0;
    for (uint8_t containerIndex = 0; containerIndex < foundDescriptorAmount;
            containerIndex++) {
        PANDescriptor *currentDescriptorPtr =
                &panDescriptorContainer[containerIndex];
        if (currentDescriptorPtr->panId == panId) {
            currentLinkValue = (1
                    - static_cast<float>(currentDescriptorPtr->rcCoord / 0xFFFF))
                    + static_cast<float>(currentDescriptorPtr->linkQuality / 0xFF);
            if (currentLinkValue > bestLinkValue) {  // (shortAddress === 0) ||
                bestLinkValue = currentLinkValue;
                resultDescriptorPtr = currentDescriptorPtr;
            }
        }
    }
    return resultDescriptorPtr;
}

/***
 * Short: panDescriptorContainer String output
 * Description: This method transforms the panDescriptor container into string-format with \n as line ending
 * @return String description of panDescriptorContainer
 * Example:
 * Discovery Container Results:
 * Network ID: 1001  LBA Agent: 0000 Link Quality: 6300 RoutingCost: 00A2
 * Network ID: 0000  LBA Agent: 0000 Link Quality: 0000 RoutingCost: 0000
 * Network ID: 0000  LBA Agent: 0000 Link Quality: 0000 RoutingCost: 0000
 * Network ID: 0000  LBA Agent: 0000 Link Quality: 0000 RoutingCost: 0000
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
char* ModemController::PANDescriptorContainerToString(char *destBuff, int outLen, uint8_t containerIndex) {
    if (containerIndex == 0) {
        snprintf(destBuff, outLen, TX::getText(TX::cTxtDiscoveryConResults));
    } else {
        *destBuff = 0;
    }
    if (containerIndex < foundDescriptorAmount) {
        char tmpBuff[100];
        PANDescriptor descriptor = panDescriptorContainer[containerIndex];
        snprintf(tmpBuff, sizeof(tmpBuff), TX::getText(TX::cTxtDiscoveryContainerN),
                descriptor.panId, descriptor.adress, descriptor.linkQuality,
                descriptor.rcCoord);
        strncat(destBuff, tmpBuff, outLen);
    }
    return destBuff;
}

/***
 * Short: Clear all stored values in the panDescriptorContainer
 */
void ModemController::resetDiscoverySearchResults() {
    for (uint8_t containerIndex = 0; containerIndex < maxPANDescriptorSets;
            containerIndex++) {
        for (uint8_t itemIndex = 0;
                itemIndex < (sizeof(PANDescriptor) / sizeof(uint16_t));
                itemIndex++) {
            panDescriptorContainer[containerIndex].discriptorItem[itemIndex] =
                    0;
        }
    }
}
#endif

/***
 * This method activates the coordinator as a network member on position 0
 * in the network list
 */
void ModemController::activateCoordinatorWithJoinConfirmation() {
    networkMemberList[0] = NetworkMember(0, 0);
    networkMemberList[0].setLink(NetworkMember::connected);
}

/***
 * This method updates the PAN and IP address of the network member
 * at position 1, who is the representative of the modemController
 * @param confirmation  incoming PLC information
 */
void ModemController::updateAddressInfoWithJoinParameter(uint16_t ip,
                                                         uint16_t panID) {
  networkMemberList[1].setNetworkIPAddress(ip);
  networkMemberList[1].setNetworkPanAddress(panID);
}

// TODO(AME) Function header
char* ModemController::monitorAliveStatusAndGetLog() {
    static const uint8_t arraySize = 5;
    static uint32_t medianArray[arraySize] = { 0 };
    static uint8_t medianIndex = 0;
    static uint32_t maxTime = 0;
    static uint32_t minTime = 0xFFFF;
    TimeMeasurement::measureJob &job = *ProgramConfig::getAliveRoundJobPtr();
    medianArray[medianIndex] = TimeMeasurement::getJobDeltat(job);
    TimeMeasurement::resetJobTime(job);

    // Calculation of Max and Min round trip time
    if (medianArray[medianIndex] > maxTime) {
        maxTime = medianArray[medianIndex];
    } else if (medianArray[medianIndex] < minTime) {
        minTime = medianArray[medianIndex];
    }

    double median = static_cast<double>(medianArray[0]);
    for (uint8_t medianCalcIndex = 1; medianCalcIndex < arraySize; medianCalcIndex++) {
        median += static_cast<double>(medianArray[medianCalcIndex]);
    }
    median /= arraySize;

    if (medianIndex++ > arraySize) {
        medianIndex = 0;
    }

    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtAliveMonitorLog),
            medianArray[medianIndex], median, maxTime, minTime);
    return Global2::OutBuff;
}

#if ALIVE_CHECK_LEVEL == 1
void ModemController::checkNetworkMembers() {
    typedef enum {
        sendAliveCheck,
        proofSendConfirmation,
        queueBreak,
        waitAnswer,
        deleteMemberEntry
    } memberCheckSteps;
    static char aliveMessage[4] = {ModemInterpreter::commandPrefix[0], ModemInterpreter::commandPrefix[1], 'A', 0};
    static int networkIndex = 1;
    static int checkCount = 0;
//  static uint32_t roundTripStartTime=0;
    static uint8_t waitCount = 0;
    static uint8_t callOutCount = 0;
    const uint8_t waitTime = 10;
    const uint8_t timeOut = 3;
    static uint8_t timeOutArray[networkMemberMax]={};

    static memberCheckSteps memberCheckStep = sendAliveCheck;
    NetworkMember *selectedMember;

    if (!isAliveCheckEnabled()) {
        return;
    }

    if (isNewMemberRequest(NMM_ALIVE)) {
        newMemberRequest &= ~NMM_ALIVE;
        memberCheckStep = sendAliveCheck;
        aliveSendFailCount = 0;
        // waitCount = 0;
        networkIndex = 1;
        lastCheckIndex = -1;
        return;
    } else {
        checkMemberPending = false;
        networkIndex = findNextAktivMemberIndexFromPosition(networkIndex);
        if (networkIndex > 0) {
            selectedMember = &networkMemberList[networkIndex];
            if (selectedMember->isAktiv()) {
                switch (memberCheckStep) {
                    case sendAliveCheck:
                        aliveTransmitIndex = networkIndex;
                        ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(
                                aliveMessage, aliveTransmitIndex, false);
                        timeOutArray[networkIndex] = 0;
                        memberCheckStep = proofSendConfirmation;
                        break;
                    case proofSendConfirmation:
                        if (ModemInterpreter::getLastDataStatusCode()== 0xFF) {
                            memberCheckStep = proofSendConfirmation;
                        } else if (ModemInterpreter::getLastDataStatusCode() == 0) {
                            memberCheckStep = waitAnswer;
                        } else if (ModemInterpreter::getLastDataStatusCode() == 0x92) {
                            memberCheckStep = queueBreak;
                        } else if ((ModemInterpreter::getLastDataStatusCode() & 0x80) == 0) {  // -> >0?
                            memberCheckStep = sendAliveCheck;
                            if (aliveSendFailCount++ > 5) {
                                networkMemberList[aliveTransmitIndex].setIsAktiv(false);
                                ModemInterpreter::modemCrierReff->println(TX::getText(TX::cTxtAliveTimeout));
                                ModemInterpreter::modemCrierReff->println(
                                        networkMemberList[aliveTransmitIndex].toString());
                                aliveSendFailCount = 0;
                                aliveTransmitIndex = 0;
                                return;
                            } else {
                                ModemInterpreter::modemCrierReff->println(TX::getText(TX::cTxtAliveTransError));
                                ModemInterpreter::modemCrierReff->println(
                                        networkMemberList[aliveTransmitIndex].toString());
                            }
                        }
                        break;
                    case waitAnswer:
                        if (lastCheckIndex == networkIndex) {
                            timeOutArray[networkIndex] = 0;
                            if (++checkCount >= aktivNetMemberCount) {
                                checkCount = 0;
                                // roundTripTime = MainLoop::getLoopTime();
                                // MainLoop::setLoopTime(0);
                            }
                            if (++networkIndex > networkMemberMax) {
                                networkIndex = 1;
                            }
                            lastCheckIndex = -1;
                            memberCheckStep = sendAliveCheck;
                            waitCount = 0;
                            callOutCount = 0;
                        }
                        if (waitCount++ > waitTime) {
                            waitCount = 0;
                            if (++timeOutArray[networkIndex] > timeOut) {
                                ModemInterpreter::modemCrierReff->println(TX::getText(TX::MemberDoesNotResponse));
                                ModemInterpreter::modemCrierReff->println(networkMemberList[networkIndex].toString());
                                networkMemberList[aliveTransmitIndex].setIsAktiv(false);
                            } else {
                                ModemInterpreter::modemCrierReff->printFormatMessageWithNumber(
                                                TX::getText(TX::MemberWithIPNoResponse), networkIndex);
                                ModemInterpreter::modemCrierReff->println(TX::getText(TX::TxtContinue));
                            }
                            networkIndex++;
                            memberCheckStep = sendAliveCheck;
//                          if (callOutCount++ > timeOut) {
//                              callOutCount = 0;
//                              waitCount = 0;
//                              memberCheckStep = sendAliveCheck;
//                              networkMemberList[aliveTransmitIndex].setIsAktiv(false);
//                              ModemInterpreter::modemCrierPtr->println("Teilnehmer meldet sich nicht (Timeout):");
//                              ModemInterpreter::modemCrierPtr->println(networkMemberList[aliveTransmitIndex].toString());
//                              aliveSendFailCount = 0;
//                              aliveTransmitIndex = 0;
//                          } else {
//                              ModemInterpreter::modemCrierPtr->println("Alive Zeit Überschreitung sende Erneut:");
//                              ModemInterpreter::modemCrierPtr->println(networkMemberList[aliveTransmitIndex].toString());
//                              memberCheckStep = sendAliveCheck;
//                          }
                        }
                        break;
                    case queueBreak:
                        if (waitCount++ > waitTime) {
                            waitCount = 0;
                            memberCheckStep = sendAliveCheck;
                        }
                        break;
                }
            } else if (++networkIndex > networkMemberMax) {
                networkIndex = 1;
            }
        } else {
            networkIndex = 1;
        }
    }
}
#endif

char* ModemController::showNetworkMembers(__attribute__((unused)) char *text) {
    memberListDisplayIndex = ProgramConfig::isCoordinator() ? 0 : 1;
    actualDisplayState = displayMembersStart;
    ModemInterpreter::setInterpretState(ModemInterpreter::showMembers);
    return const_cast<char *>("");
}

void ModemController::resetModem() {
    // modemInterface->getSciPtr()->changeBaudRate(baud115200);
    cpxResetPinPtr->setLevel(PortManager::Low);
    ModemBootLoader::setFirmenwareStatusUnkown(true);
    for (int i = 0; i < 1000000; i++) {
        asm("nop");
    }
    cpxResetPinPtr->setLevel(PortManager::High);
}

NetworkMember* ModemController::findNetworkmemberWithMacAddress(
        const uint8_t *macAddress) {
    uint16_t netMemberPosition = findNetworkmemberPositionWithMacAddress(
        macAddress);
    return &networkMemberList[netMemberPosition];
}

uint16_t ModemController::getNetMemberIndexForShortAddress(
        uint16_t targetShortAddress) {
    for (int memberIndex = 1; memberIndex < networkMemberMax + 1;
            memberIndex++) {
        uint16_t memberShortAddress = uint8Operations::sum2BytesFromLSB(
            networkMemberList[memberIndex].getMacAddress() + 6);
        if (memberShortAddress == targetShortAddress) {
            return (uint16_t) memberIndex;
        }
    }
    return 0;
}

uint8_t ModemController::countActivNetworkMembers() {
    uint8_t memberCount = 0;
    for (int i = 0; i <= networkMemberMax; i++) {
        if (networkMemberList[i].getLink() == NetworkMember::connected) {
            memberCount++;
        }
    }
    return memberCount;
}

/***
 * This Method disables a Net Member after he could not Join into the Network.
 * This is necessary in order to free network-IPs for other join-candidates.
 */
void ModemController::disableNetMemberAfterPendingTimeout() {
    for (int i = 1; i <= networkMemberMax; i++) {
        if ((networkMemberList[i].getLink() == NetworkMember::pending)) {
            updateMemberIndexPendTime((uint16_t) i);
        }
    }
}

/***
 * This method updates the pending time for a row in the networkMemberList field.
 * @param memberIndex => Index of the networkMemberList field
 */
void ModemController::updateMemberIndexPendTime(uint16_t memberIndex) {
    uint8_t netMemberPendingTime =
            networkMemberList[memberIndex].getPendingTime();
    if ((netMemberPendingTime != NetworkMember::timeNotPending)) {
        if (--netMemberPendingTime == 0) {
            disableNetMemberAtIndex(memberIndex, true, true);
        } else {
            networkMemberList[memberIndex].setPendingTime(netMemberPendingTime);
        }
    }
}

/***
 * This method disable a row in the networkMemberList field.
 * Further more it gives a info-message for the User
 * @param index => Index of the networkMemberList field
 * updateClientInfo => true: update the client information in the modem. false: client information is not updated.
 * Caution: if updateClientInfo is true this method must not be called in a loop due the client information is
 * transmitted in a separate process. Please use this method instead once at at a time within several 100ms intervals.
 */
void ModemController::disableNetMemberAtIndex(uint16_t index,
        bool updateClientInfo, bool verbose) {
    NetworkMember *pn = getNetMemberPtrByIndex(index);
    if (pn && (!pn->getTestMember())) {
        uint16_t prev_ip = pn->getNetworkIPAddress();
        pn->setLink(NetworkMember::disconnected);
        pn->setOutgoingRoute(NetworkMember::unknown);
      pn->setNetworkPanAddress(0xff);
      pn->setNetworkIPAddress(0xff);
        pn->setPendingTime(NetworkMember::timeNotPending);
        updateStorageClientInfoByIdx(index);
        if (updateClientInfo && prev_ip <= networkMemberMax) {  // Disable Client Info on the Modem
            triggerSinglePlcRequestWithArgs(setClientInfo, 4, channel0, false,
                    prev_ip, (uint32_t) pn->getMacAddress());
        }
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtMemberIPDeactivated),
                prev_ip);
    } else {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::DeactivationRejected), index);
    }
    if (verbose) {
        termFilterPtr_->println(Global2::OutBuff);
    }
}

// Coordinator: Kick out peers who don't report back after a force join
#pragma GCC diagnostic ignored "-Wstack-usage="
List<PlcCommand>& ModemController::disableNotConnectedPeers() {
    CHANNEL id = channel0;
    uint16_t prev_ip;
    const uint8_t *extAdr;
    List<PlcCommand> &delList = nodeCmdDefaultList;
    delList.clear();
    NetworkMember *pn;
    for (uint32_t i = 1; i <= ModemController::getNetworkMemberMax(); i++) {
        pn = ModemController::getNetMemberPtrByIndex((uint16_t) i);
        if (pn != NULL) {   // && pn->getOutgoingRoute()
            NetworkMember::linkStatus link = pn->getLink();
            if (link == NetworkMember::notVerified) {
                prev_ip = pn->getNetworkIPAddress();
                extAdr = pn->getMacAddress();
                // ToDo(AME): adjust if i>sizeof(nodeStorageArray)
                Node<PlcCommand> &delNode = nodeStorageArray[i - 1];  // 0...6
                delNode = Node<PlcCommand>(
                        &RequestFactory::createRequest(setClientInfo, 4, id,
                                false, prev_ip, extAdr));
                delList.add(delNode);
                disableNetMemberAtIndex((uint16_t) i, false, true);
            }
        }
    }
    return delList;
}

void ModemController::reTransmittInBuff() {
    if (globalDestIP > 0) {
        ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(
                Global2::InBuff, (uint16_t) globalDestIP, false);
    }
}

uint8_t ModemController::getNextInactiveNetworkIdx() {
    uint8_t result = 0;
    uint32_t memberIndex = 0;
    NetworkMember::linkStatus selectedLink;
    while (++memberIndex <= networkMemberMax) {
        selectedLink = networkMemberList[memberIndex].getLink();
        if (selectedLink == NetworkMember::disconnected) {
            result = (uint8_t) memberIndex;
            break;
        }
    }
    return result;  // memberIndex
}

uint16_t ModemController::getNetworkIPAddress() {
    return networkMemberList[(ProgramConfig::isCoordinator() == false)].getNetworkIPAddress();
}

void ModemController::setNetworkIPAddress(uint16_t ip) {
    return networkMemberList[(ProgramConfig::isCoordinator() == false)].setNetworkIPAddress(
        ip);
}

/***
 * Return the last two bytes of the MAC address of the network member controlled by the ModemController
 * @return last two bytes of the MAC address
 */
uint16_t ModemController::getMacShortAddress() {
    return uint8Operations::sum2BytesFromLSB(
            (networkMemberList[(ProgramConfig::isCoordinator() == false)].getMacAddress()
                    + 6));
}

// Household: Finish the initialization of the network member
void ModemController::finishInitNetworkMember() {
    bool isMacAdrAvailable = i2c_app::isMacAdrAvailable();
    NetworkMember *pn = &networkMemberList[(ProgramConfig::isCoordinator() == false)];
    if (isMacAdrAvailable) {
        uint8_t macRead[8];
        i2c_app::getI2MacAdr(macRead);
        pn->fillMacAddress(macRead);
    }
    pn->setBitFullMacAddress(isMacAdrAvailable);
}

/***
 * This method sets the last two bytes of the MAC address of a network member.
 * If this member is a peer, the offset is always 1 in the network list
 * @param address   last two bytes of the MAC address
 */
void ModemController::setMacShortAddress(uint16_t macShortAddress) {
    NetworkMember *pn = &networkMemberList[(ProgramConfig::isCoordinator() == false)];
    if (!pn->hasFullMacAddress()) {
        uint8_t newMacAddress[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0xFF, 0xFF };
        uint8Operations::insertUint16intoUint8Array(newMacAddress + 1, panId);
        uint8Operations::insertUint16intoUint8Array(newMacAddress + 6, macShortAddress);
      pn->fillMacAddress(newMacAddress);
    }
}

// Show local messages on the terminal
void ModemController::handleDisplayState() {
    switch (actualDisplayState) {
    case displayNoMembers:
        termFilterPtr_->println(TX::cTxtEmptyMemberList);
        actualDisplayState = displayIdle;
        break;
    case displayMembersStart:
        termFilterPtr_->println(TX::cTxtMemberList);
        actualDisplayState = displayMembersCont;
        break;
    case displayMembersCont:
        displayNetworkList();
        break;
    case displayIdle:
        break;
    default:
        break;
    }
}

/**
 * Conversion of the register value into an SNR value
 * Description: The method converts a passed register value
 * into an SNR value (signal-to-noise ratio). This corresponds to 0x00
 * a value of -10dB and 0xFF a value of 53.75 dB. See calculation
 * Reference: Renesas G3-PLC Frequently Asked Questions page 8
 * @param LQIvalue
 * @return
 */
float ModemController::calculateLinkQualityinDB(uint8_t LQIvalue) {
    const float pitch = 0.25;
    const int8_t linkQualityOffset = -10;
    return pitch * LQIvalue + linkQualityOffset;
    /*
     * Calculation
     * 0X00 corrosponds -10 dB
     * 0xFF corrosponds 53.75 dB
     * The values ​​are linearly interpolated, so it follows:
     * Inclination b = (10+53.75)/(0xFF) = 63.75/255 = 0.25
     * => y=0.25*x-10
     * 10+y=0,25x
     * 40+4y=x
     */
}

// TODO(AME): move to the alive flow
/**
 * This Method monitors the expiring time between an
 * outgoing Alive Transmission and an incoming Alive
 * Transmission for the following Peer.
 * If a certain threshold is reached an exception
 * function is triggered.
 */
/***
void ModemController::checkAliveTimeOut() {
    enum {
        _50s = 1000
    };
    uint32_t timeOutDelta = TimeMeasurement::getJobDeltat(*aliveTimeOutJobPtr);
    if (timeOutDelta > _50s) {
        uint8_t &callOut = getAliveCallOutCounterReff();
        termFilterPtr_->println("Alive Time Out!");
        if (callOut++ >= 3) {
            termFilterPtr_->println("Alive Call Out!");
            TimeMeasurement::unregisterTimedJob(*aliveTimeOutJobPtr);
            callOut = 0;
        }
        TimeMeasurement::resetJobTime(*aliveTimeOutJobPtr);
        //triggerSendBroadCastDataWithLengthToGroupeIP((uint8_t*)ModemInterpreter::getAliveChainMessagePtr(),9, getAliveNextGroupe());
    }
}
***/

/*** This method returns a pointer to a message of a PLC data command.
 *
 * @param confirmationPtr  to be examined PLC data command
 * @return
 */
const uint8_t* ModemController::getMessagePtrFromPlcCommand(
        const PlcCommand &confirmationPtr) {
    enum {
        nsduLengthOffset = 2
    };
    const uint8_t *nsduPtr = confirmationPtr.getParameter();
    nsduPtr += UDPFrame::ipv6HeaderLength + sizeof(UDPFrame::UDPformat)
            + nsduLengthOffset;
    return nsduPtr;
}

void ModemController::transmitt(const uint8_t *message, uint8_t length,
                                uint16_t adressIdentifer, char options[5]) {
  triggerSendBroadCastDataWithLengthToGroupeIP(message, length, adressIdentifer, false, true);
}


}  // namespace AME_SRC

