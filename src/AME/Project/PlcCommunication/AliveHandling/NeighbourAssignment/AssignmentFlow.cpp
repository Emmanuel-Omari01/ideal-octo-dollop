/*
 * AssignmentFlow.cpp
 *
 *  Created on: Aug 8, 2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "AssignmentFlow.h"

#include <cstdlib>
#include <cstdarg>

#include "../../../SystemBase/Global2.h"
#include "../../ModemInterpreter.h"
#include "../../../HelpStructures/CharOperations.h"
#include "../MessageForwarding/BroadcastRoutingManager.h"
#include "../../../SystemBase/ProgramConfig.h"
#include "../../../SystemLogic/StorageAdministrator.h"
#include "../../../Terminal/SerialProfiles/SerialStandardProfile.h"

namespace AME_SRC {

AdjazenzParent *AssignmentFlow::adjazenzParentPtr_ = NULL;
TermContextFilter *AssignmentFlow::termFilterPtr_ = NULL;

bool AssignmentFlow::processing = false;
// uint16_t AssignmentFlow::ipCounter= 0;
uint8_t AssignmentFlow::flowStep = idle;  // TODO(TH): find solution for the test automatization => waitAutomaticA
uint8_t AssignmentFlow::flowSubStep = idle;
bool AssignmentFlow::automaticMode = false;
uint8_t AssignmentFlow::automaticStatus = eStatus::none;

uint8_t AssignmentFlow::routingMode = unknown;
uint8_t AssignmentFlow::numMinPeers = 1;
uint8_t AssignmentFlow::cmpAssignedPeers = 0;
uint8_t AssignmentFlow::assignLoopCallOuts = 0;
uint8_t AssignmentFlow::assignPeerIdx = 0;
uint8_t AssignmentFlow::peerIgnoreAssignRequests = 0;
uint16_t AssignmentFlow::transmissionIPBeforeEOB = 0;

AssignmentFlow::intersectionBypassInfo AssignmentFlow::routeBypassInfo[10] = {
        0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
AssignmentFlow::TupelContext AssignmentFlow::tmpTupelContext[maxContexts] = { {
NULL, 0, 0 }, { NULL, 0, 0 } };

AssignmentFlow::AssignmentFlow(SerialDriver &serialDriv, LEDSignalFlow &ledFlow) :
        ledFlowRef_ { ledFlow } {
    termFilterPtr_ = new TermContextFilter(&serialDriv);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);

    adjazenzParentPtr_ = new AdjazenzParent(&serialDriv);
    // TODO(AME) Auto-generated constructor stub
}

AssignmentFlow::~AssignmentFlow() = default;

// @return: true: wait until next assignment
bool AssignmentFlow::doAssignment(enum eDoAssignMode mode) {
    // Route
    //  Route 0 -> 1
    //  Route 1 -> 3
    //  Route 3 -> 1
    //  Route 1 -> 0
    //  Route 0 -> 2
    //  Route 2 -> 0

    // Descriptions of roles
    // 0x87 = Administrator  / 0xFF = Supervisor / 0x00 = Transmitter / 0x01 = Receiver / 0x02 = Listener

    switch (mode) {
    case edaStart: {
        assignPeerIdx = 0;
        mode = edaPreCalculation;
        break;
    }
    case edaPreCalculation: {
        storeDataForBypassingEOBIntersection();
        printEOBBypassingResults();
        termFilterPtr_->println(Global2::OutBuff);
        break;
    }
    case edaNext:
        assignPeerIdx++;
        break;
    default:
        break;
    }

    while (processing) {
        int temp = isMemberIndexCoordinatorOrConnected(assignPeerIdx);
        switch (temp) {
        case isMember: {
            uint16_t targetIP = ModemController::getNetMemberIPByIdx(
                    assignPeerIdx);
            IRoutingManager::MemberIpTupel *comRoles =
                    getCommunicationRolesForPeerIP(targetIP);
            if (comRoles->ip == 0xFFFF) {
                processing = false;
                assignPeerIdx = 0;
                return false;
            }
            transferCommunicationRolesToIP(comRoles, targetIP);
            // netIndex++;
            return (targetIP != 0);
        }
        case notConnected:
            ModemController::disableNetMemberAtIndex(assignPeerIdx, true);
            assignPeerIdx++;
            break;
        default:    // max
            processing = false;
            // netIndex = 0;
            return false;
        }
    }
    return false;
}

IRoutingManager::MemberIpTupel* AssignmentFlow::getCommunicationRolesForPeerIP(
        uint16_t peerIP) {
    uint8_t orderPositions[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF };

    fillPeerOrderPositionArray(peerIP, orderPositions, sizeof(orderPositions));
    IRoutingManager::MemberIpTupel *comRoles =
            getCommunicationRolesByPositionAndIp(orderPositions, peerIP);
    editeRolesIfPeerIPAtRoundTripEnd(comRoles, peerIP);
    return comRoles;
}

void AssignmentFlow::editeRolesIfPeerIPAtRoundTripEnd(
        IRoutingManager::MemberIpTupel *comRoles, uint16_t peerIP) {
    AdjazenzParent::tuple *lastPeer = adjazenzParentPtr_->getRoute()
            + adjazenzParentPtr_->getRouteListEntries(true);
    AdjazenzParent::tuple *prelastPeer = adjazenzParentPtr_->getRoute()
            + (adjazenzParentPtr_->getRouteListEntries(true) - 1);
    AdjazenzParent::tuple *preprelastPeer = adjazenzParentPtr_->getRoute()
            + (adjazenzParentPtr_->getRouteListEntries(true) - 2);
    uint8_t lengthIndex = 0;
    while (comRoles[lengthIndex].ip != 0xFFFF) {
        lengthIndex++;
    }
    lengthIndex--;
    if (preprelastPeer->source == peerIP) {
        comRoles[lengthIndex].ip = IRoutingManager::BGEnd;
    }
    if (prelastPeer->source == peerIP) {
        if (prelastPeer->source == 0) {
            comRoles[lengthIndex - 2].meberID = IRoutingManager::notUsedID;
            comRoles[lengthIndex - 2].ip = 0;
        } else {
            comRoles[lengthIndex - 2].ip = IRoutingManager::BGEnd;
            comRoles[lengthIndex - 1].ip = IRoutingManager::BGAdmin;
        }
        comRoles[lengthIndex].ip = 0;
        comRoles[lengthIndex].meberID = IRoutingManager::notUsedID;
    }
    if (lastPeer->source == peerIP) {
        comRoles[lengthIndex - 3].meberID = IRoutingManager::Transmitter;
        comRoles[lengthIndex - 3].ip = IRoutingManager::BGEnd;
        comRoles[lengthIndex - 2].ip = IRoutingManager::BGEndAck;
        comRoles[lengthIndex].ip = 0;
        comRoles[lengthIndex].meberID = IRoutingManager::notUsedID;
    }

    if (peerIP == 0) {
        comRoles[0].meberID = IRoutingManager::Administrator;
        comRoles[0].ip = IRoutingManager::BGEnd;
    }
}

void AssignmentFlow::fillPeerOrderPositionArray(uint16_t peerIP, uint8_t *positionArray, uint8_t size) {
    uint16_t orderEntries = adjazenzParentPtr_->getRouteListEntries(true) + 1;
    AdjazenzParent::tuple *treeNavigationOrder = adjazenzParentPtr_->getRoute();
    uint8_t posArrayIndex = 0;
    for (uint16_t orderIndex = 0; orderIndex < orderEntries; orderIndex++) {
        if (peerIP == treeNavigationOrder->source) {
            // || ((peerIP == treeNavigationOrder->destination) && (orderIndex==orderEntries-1)))
            positionArray[posArrayIndex] = (uint8_t) orderIndex;
            if (posArrayIndex < size) {
                posArrayIndex++;
            }
        }
        treeNavigationOrder++;
    }
}

//  Member division                                     loss option
//  coord  {0x87;0x0004}{0x00;0x0001}{0xFF;0x0002}
//  Peer 1 {0x01;0x0001}{0x00;0x0002}{-----------}
//  Peer 3 {0x01;0x0002}{0x00;0x0003}{0xFF;0x0004}      {0x02,0x0001}
//  Peer 1 {0x01;0x0003}{0x00;0x0004}{0xFF;0x0005}
//  Koord  {0x01;0x0004}{0x00;0x0005}{0xFF;0x0006}      {0x02,0x0003}

// -----------------------------------------------
// Route 0 -> 1
// Route 1 -> 0
// Route 0 -> 2
// Route 2 -> 0
// Route 0 -> 3

// {role ;Broadcast group} // 0xFF := global groups
// Legend // 0x01 := Transmitter // 0x00 := Receiver   // 0xFF := Supervisor

//  posIndex
//  0       Koord  {0x00;0x0000}{0x01;0x0001}{0xFF;0x0002}
//  1       Peer 1 {0x00;0x0001}{0x01;0x0002}{0xFF;0x0003}
//  2       Koord  {0x00;0x0002}{0x01;0x0003}{0xFF;0x0004}
//  3       Peer 2 {0x00;0x0003}{0x01;0x0004}{0xFF;0x0005}
//  4       Koord  {0x00;0x0004}{0x01;0x0005}{0xFF;0x0006}
//  5       Peer 3 {0x00;0x0005}{0x01;0x0006}{0xFF;0x0007}

// back
// legende 0x87 := Administrator  // 0x? := SectionGuard
//  Koord  {0x01;0x0001}{0x00;0xFF}{0xFF;0x0003} (first row )
//  Peer 3 {0x?;0xFF}{0x00;0x0007}{0xFF;0x0008}
// Position Array for coordinator [0,2,4]

IRoutingManager::MemberIpTupel* AssignmentFlow::getCommunicationRolesByPositionAndIp(
        uint8_t *positionArray, uint16_t peerIP) {
    TupelContext *pContext = getTupelContext(create);
    MemberRolesProcessContext roleGetProcess = { peerIP, pContext->pTupelArray,
            0, 2, 0, 0, 0 };
    while (positionArray[roleGetProcess.routeOrderIdx] != 0xFF) {
        roleGetProcess.transmitterGroup = positionArray[roleGetProcess.routeOrderIdx];
        fillMemberRollsForProcessContext(roleGetProcess);
        roleGetProcess.routeOrderIdx++;
    }
    solveDoubleRolesForProcessContext(roleGetProcess);
    clearListenerRolesForEOB(roleGetProcess, positionArray);
    roleGetProcess.memberRoles[roleGetProcess.rolesOffset].ip = 0xFFFF;
    return roleGetProcess.memberRoles;
}

void AssignmentFlow::fillMemberRollsForProcessContext(MemberRolesProcessContext &processContext) {
    uint8_t *rolesPtr = getRolesForProcessContext(processContext);
    mapRolesToGroupIpsForProcessContext(rolesPtr, processContext);

    processContext.rolesOffset += processContext.roleAmount;
}

uint8_t* AssignmentFlow::getRolesForProcessContext(
        MemberRolesProcessContext &processContext) {
    static uint8_t level1Roles[3] = { IRoutingManager::Receiver,
            IRoutingManager::Transmitter, IRoutingManager::Supervisor };
    static uint8_t level2Roles[5] = { IRoutingManager::Receiver,
            IRoutingManager::Transmitter, IRoutingManager::Supervisor,
            IRoutingManager::ListenerF1, IRoutingManager::ListenerB1 };
    uint8_t *rolesPtr = 0;
    switch (processContext.roleAssignmentLevel) {
    case 1:
        rolesPtr = level1Roles;
        processContext.roleAmount = 3;
        break;
    case 2:
        rolesPtr = level2Roles;
        processContext.roleAmount = 5;
        break;
    case 3:
        // rolesPtr = level3Roles;
        processContext.roleAmount = 7;
        break;
    }
    return rolesPtr;
}

void AssignmentFlow::mapRolesToGroupIpsForProcessContext(uint8_t *roles,
        MemberRolesProcessContext &processContext) {
    for (uint8_t roleIndex = 0; roleIndex < processContext.roleAmount; ++roleIndex) {
        // Broadcast group is counted up based on position in the order
        uint8_t &targetMemberID =
                processContext.memberRoles[processContext.rolesOffset + roleIndex].meberID;
        targetMemberID = roles[roleIndex];

        uint16_t &targetIP =
                processContext.memberRoles[processContext.rolesOffset + roleIndex].ip;
        targetIP = determineIpForRoleInProcessContext(roles[roleIndex], roleIndex, processContext);
        if (targetIP == 0 || targetIP == 0xFFFF) {
            targetMemberID = IRoutingManager::notUsedID;
            targetIP = 0;
        }
    }
}

uint16_t AssignmentFlow::determineIpForRoleInProcessContext(uint8_t targetRole,
        uint8_t roleOffset, MemberRolesProcessContext &processContext) {
    uint16_t targetIP = 0;
    if (targetRole == IRoutingManager::ListenerF1) {
        int ipValue = processContext.transmitterGroup
                - processContext.roleAssignmentLevel + 1;
        if (routeBypassInfo[0].receiverIP == processContext.currentIP) {
            ipValue = routeBypassInfo[0].transmitterGroup;
        }
        targetIP = (uint16_t) ipValue;

    } else if (targetRole == IRoutingManager::ListenerB1) {
        targetIP = (uint16_t) (processContext.transmitterGroup
                + processContext.roleAssignmentLevel + 1);
        if (routeBypassInfo[0].transmitterIP == processContext.currentIP) {
            targetIP = routeBypassInfo[0].receiverGroup;
        }
    } else {
        targetIP = (uint16_t) (processContext.transmitterGroup + roleOffset);
    }
    return targetIP;
}

void AssignmentFlow::solveDoubleRolesForProcessContext(
        MemberRolesProcessContext &processContext) {
    for (uint16_t firstRoleIdx = 0; firstRoleIdx < processContext.rolesOffset;
            ++firstRoleIdx) {
        for (uint16_t secondRoleIdx = firstRoleIdx;
                secondRoleIdx < processContext.rolesOffset; ++secondRoleIdx) {
            if (firstRoleIdx == secondRoleIdx) {
                continue;
            }
            IRoutingManager::MemberIpTupel &first =
                    processContext.memberRoles[firstRoleIdx];
            IRoutingManager::MemberIpTupel &second =
                    processContext.memberRoles[secondRoleIdx];
            if (second.ip != 0 && first.ip == second.ip) {
                if (first.meberID == IRoutingManager::Transmitter) {
                    second.ip = 0;
                    second.ip = IRoutingManager::notUsedID;
                    first.meberID = IRoutingManager::EndTransmitter;
                } else {
                    first.ip = 0;
                    first.meberID = IRoutingManager::notUsedID;
                }
            }
        }
    }
}

void AssignmentFlow::clearListenerRolesForEOB(
        MemberRolesProcessContext &processContext, uint8_t *positionArray) {
    enum {
        ListenerF1Offset = 3, ListenerB1Offset = 4, startOffset = 3
    };
    if (processContext.routeOrderIdx != 1) {
        return;
    }

    if (positionArray[0] < startOffset) {
        return;
    }

    AdjazenzParent::tuple *EOBcandidate = adjazenzParentPtr_->getRoute()
            + positionArray[0];

    bool isEndOfBranchFirst = ((EOBcandidate - 3)->source
            == (EOBcandidate - 2)->destination);
    bool isEndOfBranchSecond = ((EOBcandidate - 1)->source
            == EOBcandidate->destination);
    if (isEndOfBranchFirst && isEndOfBranchSecond) {
        processContext.memberRoles[ListenerF1Offset].meberID = IRoutingManager::notUsedID;
        processContext.memberRoles[ListenerF1Offset].ip = 0;
        processContext.memberRoles[ListenerB1Offset].meberID = IRoutingManager::notUsedID;
        processContext.memberRoles[ListenerB1Offset].ip = 0;
    }
}

uint8_t AssignmentFlow::getCommunicationRoleAmountFromRoles(
        IRoutingManager::MemberIpTupel *pTupelArray) {
    uint8_t count = 0;
    while (pTupelArray[count].ip != 0xFFFF && count < IRoutingManager::maxTupelAmount) {
        count++;
    }
    return count;
}

bool AssignmentFlow::isTupelCapable(IRoutingManager::MemberIpTupel *pt) {
    return (pt->ip != 0) && (pt->meberID != IRoutingManager::notUsedID);
}

uint8_t AssignmentFlow::removeNotCapableTupels(uint8_t comRoleAmount,
        IRoutingManager::MemberIpTupel *comRoles) {
    uint8_t newAmount = 0;
    if (comRoleAmount != 0) {
        uint32_t checkAmount = (uint32_t) (comRoleAmount - 1);
        for (uint32_t i = 0; i < comRoleAmount; i++) {
            if (isTupelCapable(&comRoles[i])) {
                newAmount++;
            } else {
                for (uint32_t j = i; j < checkAmount; j++) {
                    comRoles[j] = comRoles[j + 1];
                }
            }
        }
    }
    return newAmount;
}

void AssignmentFlow::transferCommunicationRolesToIP(
        IRoutingManager::MemberIpTupel *comRoles, uint16_t targetIP) {
    TupelContext *pTransfer = getTupelContext(transfer);
    uint8_t comRoleAmount = getCommunicationRoleAmountFromRoles(comRoles);
    comRoleAmount = removeNotCapableTupels(comRoleAmount, comRoles);
    printTransferInformation(comRoleAmount, comRoles, targetIP);
    if (targetIP == 0) {
        createTransMessageFromTupels(Global2::OutBuff, comRoles, comRoleAmount);
        // ModemInterpreter::setNumBGOffset(offset);
        IRoutingManager::MemberIpTupel *source = comRoles;
        IRoutingManager::MemberIpTupel *dest = pTransfer->pTupelArray;
        for (int i = 0; i < comRoleAmount; i++) {
            *dest++ = *source++;
        }
        pTransfer->numTupelEntries = comRoleAmount;
        pTransfer->ofsRegModem = 1;
        ModemInterpreter::setInterpretState(ModemInterpreter::triggerBGroupSet);
    } else {
        createBGSingleTransmission((uint8_t) targetIP, comRoles, 1, 0,
                (uint8_t) comRoleAmount);
    }
}

void AssignmentFlow::printTransferInformation(uint8_t comRoleAmount,
        IRoutingManager::MemberIpTupel *comRoles, uint16_t targetIP) {
    if (targetIP == 0) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::dTxtAutomatic001), comRoleAmount);
    } else {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::dTxtAutomatic002), comRoleAmount, targetIP);
    }
    termFilterPtr_->println(Global2::OutBuff, TermContextFilter::filterRoutingInfo);

    uint16_t textOffset = 0;
    for (int roleIndex = 0; roleIndex < comRoleAmount; roleIndex++) {
        if (comRoles[roleIndex].ip != 0xFFFF
                && textOffset < Global2::outBuffMaxLength) {
            const char *roleNames[] = { "Receiver", "Transmitter", "ListenerF1",
                    "Supervisor", "ListenerB1", "EndTransmitter",
                    "Administrator", "notUsedID" };
            uint8_t roleNameIdx = comRoles[roleIndex].meberID;
            switch (comRoles[roleIndex].meberID) {
            case 0:
            case 1:
            case 2:
                break;
            case 0xFF:
                roleNameIdx = 3;
                break;
            case 0xFE:
                roleNameIdx = 4;
                break;
            case 0x7F:
                roleNameIdx = 5;
                break;
            case 0x87:
                roleNameIdx = 6;
                break;
            default:
                roleNameIdx = 7;
                break;
            }
            snprintf(Global2::OutBuff + textOffset, Global2::outBuffMaxLength, TX::getText(TX::ShowRoleWithIp),
                    roleNames[roleNameIdx], comRoles[roleIndex].ip);
            textOffset = (uint16_t) strlen(Global2::OutBuff);
        }
    }
    termFilterPtr_->println(Global2::OutBuff, TermContextFilter::filterRoutingInfo);
    Global2::OutBuff[0] = 0;    // suppress doubled terminal outputs
}

// @return: {eIsMemberIndex}
int AssignmentFlow::isMemberIndexCoordinatorOrConnected(uint8_t netIndex) {
    if (netIndex < ModemController::getNetworkMemberMax()) {
        NetworkMember *netMember = ModemController::getNetMemberPtrByIndex(
                netIndex);
        NetworkMember::linkStatus ls = netMember->getLink();
        if (netIndex == 0 || ls == NetworkMember::connected || ls == NetworkMember::notVerified) {
            return isMember;
        }
        return notConnected;
    }
    return max;
}

// Routine to treat
// - create coordinator routing
// - Route Discovery
// - create spanning tree
// - assignment of the broadcast groups
// - starting of the alive round trips
// Parameter:
// cause - plcMsg: call according to a message of the PLC
//       - timerBase: call of the 50ms-Task
const char* AssignmentFlow::processAssignment(enum eCallCause cause) {
    const char *msg = (const char*) "";
    static int8_t verboseBarDelay = 1;

    switch (flowStep) {
    default:
    case idle:
    case waitAutomaticB:
        break;
    case waitAutomaticA:
        if (ProgramConfig::isCoordinator()) {
            if (ProgramConfig::getTerminal()->isHelpTextDisplayed()) {
                msg = startRoutingDelay();  // Timeout starts OuterAutomaticStart
                automaticMode = true;
                setFlowStep(waitAutomaticB, const_cast<char*>(TX::getText(TX::DebugShowProcessAssignment)));
            }
        } else {
            setFlowStep(idle, const_cast<char*>(TX::getText(TX::DebugShowProcessAssignment)));
            restoreNeighbourTupels();
        }
        break;
    case OuterAutomaticStart: {
        setAutomaticState(eStatus::dwellTimeCompleted, eStatusUpdate::set);
        uint16_t actPeers = ModemController::getNumPeersConnected(false);
        uint16_t peersPending = ModemController::getNumPeersPending();
        char cNumMinPeers[8];
        snprintf(cNumMinPeers, sizeof(cNumMinPeers), "%3u", numMinPeers);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::dTxtIntegration003), cNumMinPeers);
        termFilterPtr_->println(Global2::OutBuff);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::dTxtIntegration004), actPeers);
        termFilterPtr_->println(Global2::OutBuff);
        if (peersPending) {
            Global2::OutBuff[0] = 0;
            termFilterPtr_->println(TX::dTxtAutomatic003);
            termFilterPtr_->println(
                    ModemController::getListPeersPending(Global2::OutBuff,
                            sizeof(Global2::OutBuff)));
            termFilterPtr_->println(
                    ModemController::getListPeersPending(Global2::OutBuff,
                            sizeof(Global2::OutBuff)));
        }
        if (actPeers == 0) {
            termFilterPtr_->println(TX::dTxtAutomatic004);
            setFlowStep(idle, const_cast<char*>(TX::getText(TX::DebugShowProcessAssignment)));
        } else {
            setFlowStep(OuterAutomaticLoop,
                    const_cast<char*>(TX::getText(TX::DebugShowProcessAssignment)));
        }
        break;
    }
    case OuterAutomaticLoop:
        doStartOuterTimeoutControl();
        automaticMode = true;
        setFlowSteps(routeDiscovery, startRemoteDiscovery,
                const_cast<char*>(TX::getText(TX::DebugShowProcessAssignment)));
        assignLoopCallOuts = 3;
        break;
    case routeDiscovery:
        msg = doRouteDiscovery(cause);
        break;
    case buildTree:
        msg = doBuildTree();
        break;
    case proofTree:
        msg = doProofTree();
        break;
    case sendBCOrder:
        msg = doAssignAutomatic(cause);
        break;
    case startAlive:
        msg = doStartAlive();
        break;
    }
    if (flowStep != idle && automaticMode
            && TermContextFilter::isNotFiltered(TermContextFilter::filterInitLoadBar)) {
        if (*msg) {
            termFilterPtr_->print(msg);
            msg = (const char*) "";
        }
        if (--verboseBarDelay <= 0) {
            verboseBarDelay = 20;   // 20*50ms = 1s
            termFilterPtr_->putchar('.');
        }
        if (flowStep == waitAutomaticB) {
            ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
            ITimeOutWatcher::timeOutInfo *pToi = pW->findTimeOutWithID(
                ProgramConfig::assignmentFlowTOutID);
            if (pToi && pToi->started && (pToi->timeCounter != pToi->delay)
                    && (pToi->timeCounter % 1200) == 0) {    // full minute
                uint16_t min2Go = pToi->timeCounter / 1200;  // 1200 = 60s / 0.05s
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                        TX::getText(TX::dTxtAutomatic029), min2Go);
                termFilterPtr_->print(Global2::OutBuff);
            }
        }
    }
    return msg;
}

// Select next step in the step chain
void AssignmentFlow::incFlowStep() {
    if (automaticMode) {
        switch (++flowStep) {
        default:
            break;
        case routeDiscovery:
            flowSubStep = startRemoteDiscovery;
            break;
        case buildTree:
            flowSubStep = startTree;
            break;
        case proofTree:
            flowSubStep = treeProof;
            break;
        case sendBCOrder:
            flowSubStep = startAssign;
            break;
        case startAlive:
            flowSubStep = startAliveSub;
            break;
        }
    } else {
        switch (flowStep) {
        case sendBCOrder:
            flowStep = idle;
            break;
        default:
            break;
        }
    }
    showFlowSteps(false, TermContextFilter::filterDevelope,
            const_cast<char*>(TX::getText(TX::DebugShowIncFlowStep)));
}

// Start + Refresh outer timeout control
void AssignmentFlow::doStartOuterTimeoutControl() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    ITimeOutWatcher::timeOutInfo *pToi = pW->findTimeOutWithID(
        ProgramConfig::outerAssignmentFlowTOutID);

    if (pToi == NULL) {
        ITimeOutWatcher::timeOutInfo toi;
        ITimeOutWatcher::createDefaultTimeOut(&toi,
                                              ProgramConfig::outerAssignmentFlowTOutID, &TimeOutOuterControl,
                                              NULL);
        toi.delay = 2400;           // 2400 * 0.05s = 120s = 2min
        toi.timeCounter = 2400;
        pW->registerTimeout(toi);
        termFilterPtr_->println(TX::dTxtAutomatic005, TermContextFilter::filterDevelope);
    } else {
        termFilterPtr_->println(TX::dTxtAutomatic006, TermContextFilter::filterDevelope);
        pW->resetDelayForIndex(ProgramConfig::outerAssignmentFlowTOutID);
    }
}

void AssignmentFlow::TimeOutOuterControl() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    ITimeOutWatcher::timeOutInfo *pToi = pW->findTimeOutWithID(
        ProgramConfig::outerAssignmentFlowTOutID);
    termFilterPtr_->println(TX::TimeoutOuterControl);
    if (pToi != NULL) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::ShowTimeoutCounter), pToi->timeOutCounter);
        termFilterPtr_->println(Global2::OutBuff);
    }
    if (automaticMode) {
        setFlowStep(OuterAutomaticLoop, const_cast<char*>(TX::getText(TX::TimeoutOuterControl)));
    }
}

void AssignmentFlow::storeDataForBypassingEOBIntersection() {
    bool isEOBIntersectionBefore = false;
    int orderEntries = adjazenzParentPtr_->getRouteListEntries(true);
    if (orderEntries >= 3) {
        AdjazenzParent::tuple *treeNavigationOrderPtr = adjazenzParentPtr_->getRoute();
        uint8_t bypassIndex = 0;
        for (uint16_t orderIndex = 0; orderIndex < orderEntries - 1; orderIndex++) {
            bool isEOBIntersection = treeNavigationOrderPtr->source
                    == treeNavigationOrderPtr[1].destination;
            if (isEOBIntersection) {
                if (!isEOBIntersectionBefore) {
                    isEOBIntersectionBefore = true;
                    routeBypassInfo[bypassIndex].transmitterGroup = orderIndex;
                    routeBypassInfo[bypassIndex].transmitterIP =
                            (uint16_t) ((treeNavigationOrderPtr - 1)->source);
                }
                orderIndex++;
                treeNavigationOrderPtr += 2;
            } else {
                if (isEOBIntersectionBefore) {
                    isEOBIntersectionBefore = false;
                    routeBypassInfo[bypassIndex].receiverGroup =
                            (uint16_t) (orderIndex + 1);
                    routeBypassInfo[bypassIndex].receiverIP =
                            (uint16_t) (treeNavigationOrderPtr->destination);
                    bypassIndex++;
                }
                treeNavigationOrderPtr += 1;
            }
        }
    }
}

// Peer, PLC receive of "G"-message: registration of broadcast groups. Shipping takes place via the coordinator
// Coordinator: registration of the own broadcast groups
void AssignmentFlow::addNeighbourTupels(__attribute__((unused)) const uint8_t modemRegOffset,
        const uint8_t aliveMapOffset, const uint8_t count,
        const IRoutingManager::MemberIpTupel *tupelArray) {
    IRoutingManager *pRM = ProgramConfig::getRoutingManagerPtr();
    for (int i = 0; i < count; ++i) {
        IRoutingManager::MemberIpTupel tupel = tupelArray[i];
        pRM->storeData(IRoutingManager::routingRowEntryIndex,
                (uint8_t) (i + aliveMapOffset));
        pRM->storeData(IRoutingManager::GET_IP_AT_INDEX, tupel.ip);
        pRM->storeData(IRoutingManager::GET_MEMBER_ID_AT_INDEX,
                (uint8_t) tupel.meberID);
    }
    StorageAdministrator::StoreBroadcastGroups(aliveMapOffset, count);
    // ModemController::setProcessState(ModemController::createSetGroupsWithGivenTupels(
    //                  true, modemRegOffset,aliveMapOffset, count,tupelArray).getHead());
}

// Restore force join tuples according to the broadcast groups.
// Restore both in memory and in the modem
void AssignmentFlow::restoreNeighbourTupels() {
    IRoutingManager *pRM = ProgramConfig::getRoutingManagerPtr();
    IRoutingManager::MemberIpTupel tupel;
    uint8_t count = StorageAdministrator::RestoreBroadCastGroups();
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtAutomatic008), count);
    termFilterPtr_->println(Global2::OutBuff);
    if (count) {
        TupelContext *pTransfer = getTupelContext(transfer);
        pTransfer->numTupelEntries = count;
        pTransfer->ofsRegModem = 1;
        pTransfer->ofsAliveMap = 0;
        for (int i = 0; i < count; i++) {
            pRM->storeData(IRoutingManager::routingRowEntryIndex, (uint8_t) i);
            tupel.meberID = (uint8_t) pRM->getData(
                    IRoutingManager::GET_MEMBER_ID_AT_INDEX);
            tupel.ip = pRM->getData(IRoutingManager::GET_IP_AT_INDEX);
            pTransfer->pTupelArray[i] = tupel;
        }
        // ModemController::setProcessState(ModemController::createSetGroupsWithGivenTupels(true,pTransfer->ofsRegModem,pTransfer->ofsAliveMap,count,pTransfer->pTupelArray).getHead());
    }
}

uint16_t AssignmentFlow::getRoutingEntry() {
    return ProgramConfig::getRoutingManagerPtr()->getData(
            IRoutingManager::routingRowEntryIndex);
}

void AssignmentFlow::setRoutingEntry(uint16_t value) {
    return ProgramConfig::getRoutingManagerPtr()->storeData(
            IRoutingManager::routingRowEntryIndex, value);
}

char* AssignmentFlow::triggerAutomaticSteps(char *text) {
    unsigned int cnt, numPeers;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u", &numPeers);
    if (cnt == 1) {
        uint8_t maxPeers = ModemController::getNetworkMemberMax();
        if (numPeers > maxPeers) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::NumberOfPeersForAliveInput), maxPeers);
            return Global2::OutBuff;
        }
    } else {
        numPeers = 1;
    }

    setFlowStep(OuterAutomaticStart, const_cast<char*>(TX::getText(TX::TerminalMsg)));
    setNumMinPeers((uint8_t) numPeers);
    return const_cast<char*>("");
}

char* AssignmentFlow::fillAdjMatrixTerminal(char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    bool minSpannbaum = (*txt == 'm') || (*txt == 'M');
    adjazenzParentPtr_->fillMatrixFromDiscoveryData(minSpannbaum);
    automaticMode = false;
    setFlowSteps(buildTree, showTreeStart, const_cast<char*>(TX::getText(TX::TerminalMsg)));
    return const_cast<char*>(TX::getText(TX::AdjacencyMatrixFilled));
}

char* AssignmentFlow::triggerShowNetworkDiscoveryResults(char *text) {
    int peerIP, count;
    char *msg;
    const char *txt = charOperations::skip_token(text, ' ');
    count = sscanf(txt, "%d", &peerIP);
    if (count == 1) {
        msg = ModemInterpreter::getDiscManagerPtr()->showData(txt);
    } else {
        msg = ModemInterpreter::getDiscManagerPtr()->showData("A");
    }
    termFilterPtr_->println(msg, TermContextFilter::filterRoutingInfo);
    if (ModemInterpreter::getDiscManagerPtr()->getData("status")
            == IDiscoveryManager::showDiscovery) {
        automaticMode = false;
        setFlowSteps(routeDiscovery, showRD, const_cast<char*>(TX::getText(TX::DebugShowRDResults)));
    }
    return const_cast<char*>("");
}

char* AssignmentFlow::triggerNetworkDiscovery(uint8_t select, char *text) {
    automaticMode = false;
    return ModemInterpreter::getDiscManagerPtr()->triggerDiscovery(select, text);
}

// coordinator, Test via terminal command: transmission of 1 or more broadcast groups to 1 peer
// Peer, Test: Option to ignore broadcast group assignment
char* AssignmentFlow::remoteBroadcastGroupSelection(char *text) {
    TupelContext *pTransfer = getTupelContext(transfer);
    IRoutingManager::MemberIpTupel *pTupelArray;
    enum eErrNo {
        none, parameter, tupel, notCoordinator, noTree, numIgnore
    };
    unsigned int i, tIP, offset, numTupel, dummy, nIP, nMemberId;
    int cnt;
    uint32_t errNo = none;
    const char *txt = charOperations::skip_token(text, ' ');

    while (true) {
        if (!ProgramConfig::isCoordinator()) {
            cnt = sscanf(txt, "%u", &i);
            if (cnt == 1 && i < 10) {
                peerIgnoreAssignRequests = (uint8_t) i;
                errNo = numIgnore;
            } else {
                errNo = notCoordinator;
            }
        } else if (strcmp("all", txt) == 0) {
            // Set all broadcast groups for all peers and the coordinator
            if (adjazenzParentPtr_->getRouteListEntries(false) == 0) {
                errNo = noTree;
                break;
            }
            // Initialize parameter for the assignment
            ModemController::setBGtransmitMode(ModemController::processAuto);
            setFlowSteps(sendBCOrder, loopAssign, const_cast<char*>(TX::getText(TX::TerminalMsg)));
            processing = 1;
            doAssignment(edaStart);
        } else {
            cnt = sscanf(txt, "%u %u %u %u", &tIP, &offset, &numTupel, &dummy);
            txt = charOperations::skip_token(txt, ' ');  // tIP
            txt = charOperations::skip_token(txt, ' ');  // offset
            txt = charOperations::skip_token(txt, ' ');  // numTupel
            if (cnt == 4) {
                if (tIP == 0 || tIP >= ModemController::getNetworkMemberMax()
                        || offset > 15 || numTupel == 0 || numTupel > 3) {
                    // ToDo(AME): numTupel>IRoutingManager::maxTupelAmount
                    errNo = parameter;
                    break;
                }
                for (i = 0; i < numTupel && errNo == 0; i++) {
                    pTupelArray = (pTransfer->pTupelArray) + i;
                    cnt = sscanf(txt, "%u,%u", &nMemberId, &nIP);
                    if (cnt == 2) {
                        pTupelArray->meberID = (uint8_t) nMemberId;
                        pTupelArray->ip = (uint16_t) nIP;
                    } else {
                        errNo = tupel;
                        break;
                    }
                    txt = charOperations::skip_token(txt, ',');  // IP
                    txt = charOperations::skip_token(txt, ' ');  // MemberID
                }
                if (errNo) {
                    break;
                }
                createBGSingleTransmission((uint8_t) tIP,
                        pTransfer->pTupelArray, (uint8_t) offset, offset - 1,
                        (uint8_t) numTupel);
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                        TX::getText(TX::dTxtAutomatic002), numTupel, tIP);
            } else {
                errNo = 1;
            }
        }
        break;
    }

    if (errNo != none) {
        setFlowSubStep(idleAssign, const_cast<char*>(TX::getText(TX::DebugRemoteBCSelection)));
        switch (errNo) {
        case parameter:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::EnterCorrectNumberOfArgs));
            break;
       case tupel:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::TupleInputError));
            break;
        case notCoordinator:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::UseCommandInCoordinatorModeOnly));
            break;
        case noTree:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::NoSpanningTreeOrRouteInfo));
            break;
        case numIgnore:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::IgnoreBCAssignmentNTimes), peerIgnoreAssignRequests);
            break;
        default:
            break;
        }
    }
    return Global2::OutBuff;
}

char* AssignmentFlow::terminalSetFlowSteps(char *text) {
    int cnt;
    unsigned int s1, s2, autoMode;
    bool clearTO = false;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u %u %u", &s1, &s2, &autoMode);
    if ((cnt >= 1) && (s1 <= startAlive)) {
        flowStep = (uint8_t) s1;
        clearTO = true;
    }
    if ((cnt >= 2) && (s2 < 10)) {
        flowSubStep = (uint8_t) s2;
        clearTO = true;
    }
    if ((cnt == 3) && (autoMode <= 1)) {
        setAutomaticMode(static_cast<bool>(autoMode));
    }
    if (clearTO) {
        clearAllFlowTimeouts();
    }
    showFlowSteps(true, 0, const_cast<char*>(TX::getText(TX::TerminalMsg)));
    return const_cast<char*>("");
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void AssignmentFlow::showFlowSteps(bool showAlways, uint32_t mask,
        char *caller) {
    if (showAlways || termFilterPtr_->isNotFiltered((TermContextFilter::filterBits) mask)) {
        char termDump[120];
        unsigned int s1, s2, autoMode;
        autoMode = (unsigned int) automaticMode;
        s1 = (uint8_t) flowStep;
        s2 = (uint8_t) flowSubStep;
        snprintf(termDump, sizeof(termDump), TX::getText(TX::dTxtAutomatic009),
                caller, s1, s2, (autoMode) ? 'y' : 'n');
        termFilterPtr_->println(termDump);
    }
}

void AssignmentFlow::setFlowStep(uint8_t step, char *caller) {
    flowStep = step;
    showFlowSteps(false, TermContextFilter::filterDevelope, caller);
}

void AssignmentFlow::setFlowSubStep(uint8_t step, char *caller) {
    flowSubStep = step;
    showFlowSteps(false, TermContextFilter::filterDevelope, caller);
}

void AssignmentFlow::setFlowSteps(uint8_t mainStep, uint8_t subStep,
        char *caller) {
    flowStep = mainStep;
    flowSubStep = subStep;
    showFlowSteps(false, TermContextFilter::filterDevelope, caller);
}

void AssignmentFlow::clearAllFlowTimeouts() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->clearTimeout(ProgramConfig::assignmentFlowTOutID);
    pW->clearTimeout(ProgramConfig::outerAssignmentFlowTOutID);
    pW->clearTimeout(ProgramConfig::routeDiscoveryTOutID);
    termFilterPtr_->println(TX::dTxtAutomatic010, TermContextFilter::filterRoutingInfo);
}

void AssignmentFlow::createBGSingleTransmission(uint8_t tIP,
        IRoutingManager::MemberIpTupel *pTupelArray, uint8_t modemRegOffset,
        uint8_t aliveMappingOffset, uint8_t count) {
//  ITimeOutWatcher::timeOutInfo toi;

    snprintf(Global2::InBuff, Global2::inBuffMaxLength, "%s%c%03u:%u:%02u:%02u:%u",
            ModemInterpreter::commandPrefix,
            ModemInterpreter::BroadCastGroupReq, tIP, 1, modemRegOffset,
            aliveMappingOffset, count);
    createTransMessageFromTupels(Global2::InBuff, pTupelArray, count);
    strncat(Global2::InBuff, "::", sizeof(Global2::InBuff) - strlen(Global2::InBuff) - 1);

#ifdef ALIVE_PREFER_BROADCAST_TRANSMISSION
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t*>(Global2::InBuff), (uint8_t) strlen(Global2::InBuff),
            IRoutingManager::BCGlobalIP, true,
            TermContextFilter::filterRoutingInfo);
#else
    ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(Global2::InBuff, (uint8_t) tIP, false);
#endif

    ProgramConfig::getTimeOutWatcherPtr()->clearTimeout(
        ProgramConfig::rBroadcastGroupTOutID);
//  ITimeOutWatcher::createDefaultTimeOut(&toi, ProgramConfig::rBroadcastGroupTOutID,
//                  &ModemController::reTransmittInBuff, NULL);
//  ProgramConfig::getTimeOutWatcherPtr()->registerTimeout(toi);
    ModemController::setGlobalDestIp(tIP);
}

void AssignmentFlow::createTransMessageFromTupels(char *messageContainer,
        IRoutingManager::MemberIpTupel *pTupelArray, uint8_t count) {
    unsigned int i;
    char wrBuff[16];
    for (i = 0; i < count; i++) {
        snprintf(wrBuff, sizeof(wrBuff), ":%03d,%03d", pTupelArray->meberID, pTupelArray->ip);
        strncat(messageContainer, wrBuff, sizeof(messageContainer));
        pTupelArray++;
    }
}

int16_t AssignmentFlow::storeTupelsByEndingPrediction(
        IRoutingManager::MemberIpTupel *tupelsPtr, uint16_t peerIndex,
        AdjazenzParent::tuple *pTreeTupel) {
    bool isBranchEnd = (pTreeTupel[2].source == peerIndex);
    uint8_t tupelAmount = 0;
    if (isBranchEnd) {
        tupelAmount = 2;
    } else {
        tupelAmount = 3;
    }
    fillTupelTripelWithDefaultRoles(tupelsPtr);
    if (tupelAmount > IRoutingManager::maxTupelAmount) {
        return -1;
    }
    storeTupels(tupelAmount, tupelsPtr);
    return tupelAmount;
}

void AssignmentFlow::fillTupelTripelWithDefaultRoles(
        IRoutingManager::MemberIpTupel *tupelsPtr) {
    tupelsPtr[0].meberID = IRoutingManager::Transmitter;
    tupelsPtr[1].meberID = IRoutingManager::Receiver;
    tupelsPtr[2].meberID = IRoutingManager::Supervisor;
}
void AssignmentFlow::storeTupels(uint8_t length,
        IRoutingManager::MemberIpTupel *tupelsPtr) {
    TupelContext *pTransfer = getTupelContext(transfer);
    IRoutingManager::MemberIpTupel *pTupelArray = pTransfer->pTupelArray;
    for (uint8_t i = 0; i < length; i++) {
        *pTupelArray++ = tupelsPtr[i];
    }
}

char* AssignmentFlow::broadcastGroupSelection(char *text) {
    unsigned int reciverG, transmitterG, supervisorID;
    const char *txt = charOperations::skip_token(text, ' ');
    const uint8_t ModemRefOfs = 1;
    const uint8_t AliveMapOfs = 0;
    int32_t cnt = sscanf(txt, "%u %u %u", &reciverG, &supervisorID, &transmitterG);
    if (cnt == 3) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::AssignGroupsMessage),
                reciverG, supervisorID, transmitterG);
        IRoutingManager::MemberIpTupel tupelArray[] = {
                { 1, (uint16_t) reciverG },
                { 1, (uint16_t) supervisorID },
                { 1, (uint16_t) transmitterG } };
        ModemController::setProcessState(
                ModemController::createSetGroupsWithGivenTupels(true,
                        ModemRefOfs, AliveMapOfs, 3, tupelArray).getHead());
    } else {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::IncorrectEntry));
    }
    return Global2::OutBuff;
}

const char* AssignmentFlow::startRoutingDelay() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->clearTimeout(ProgramConfig::assignmentFlowTOutID);
    ITimeOutWatcher::timeOutInfo toi;
    ITimeOutWatcher::createDefaultTimeOut(&toi,
                                          ProgramConfig::assignmentFlowTOutID, &timeoutRoutingStart, NULL);
    toi.timeOutAmount = 1;
    toi.timeOutCounter = 1;
    toi.delay = 6000;           // 6000 * 0.05s = 300s = 5min
    toi.timeCounter = 6000;
    pW->registerTimeout(toi);
    return (const char*) TX::getText(TX::FiveMinuteDelayStarted);
}

void AssignmentFlow::timeoutRoutingStart() {
    setFlowStep(OuterAutomaticStart, const_cast<char*>(TX::getText(TX::DebugTimeoutRouting)));
    numMinPeers = 1;        // ToDo(AME): set an suitable numer of peers
}

void AssignmentFlow::refreshRoutingDelay() {
    if (flowStep == waitAutomaticB) {
        termFilterPtr_->println(TX::dTxtAutomatic011, TermContextFilter::filterInitLoadBar);
        ProgramConfig::getTimeOutWatcherPtr()->resetDelayForIndex(
            ProgramConfig::assignmentFlowTOutID);
    }
}



const char* AssignmentFlow::doRouteDiscovery(enum eCallCause cause) {
    const char *msg = (const char*) "";

    if (flowSubStep != idleRD) {    // 50ms
        char *timeoutMessage =
                ModemInterpreter::getDiscManagerPtr()->triggerDiscovery('5', NULL);
        if (timeoutMessage) {
            termFilterPtr_->println(timeoutMessage);
        }
    }

    switch (flowSubStep) {
    default:
    case idleRD:
        break;
    case startRemoteDiscovery:
        createRDTimeoutHandling();

        msg = ModemInterpreter::getDiscManagerPtr()->triggerDiscovery('S', NULL);
        setFlowSubStep(waitRD, const_cast<char*>(TX::getText(TX::DebugRouteDiscovery)));
        break;
    case waitRD:
        if (cause == refreshOuterTimeOut) {
            ProgramConfig::getTimeOutWatcherPtr()->resetDelayForIndex(
                ProgramConfig::outerAssignmentFlowTOutID);
            termFilterPtr_->println(TX::dTxtAutomatic012, TermContextFilter::filterDevelope);
        }
        if (ModemInterpreter::getDiscManagerPtr()->getData("status")
                == IDiscoveryManager::showDiscovery) {
            ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
            pW->clearTimeout(ProgramConfig::routeDiscoveryTOutID);
            termFilterPtr_->println(TX::dTxtAutomatic013, TermContextFilter::filterDevelope);
            setFlowSubStep(showRD, const_cast<char*>(TX::getText(TX::DebugRouteDiscovery)));
        }
        break;
    case showRD: {
        if (!termFilterPtr_->isTransmissionComplete()) {
            break;
        }
        for (int i = 0; i < 10; i++) {
            msg = ModemInterpreter::getDiscManagerPtr()->showData(NULL);
            if (*msg != 0) {
                termFilterPtr_->println(msg, TermContextFilter::filterRoutingInfo);
            }
            if (ModemInterpreter::getDiscManagerPtr()->isDataShown()) {
                setFlowSubStep((automaticMode) ? loopRD : idleRD,
                        const_cast<char*>(TX::getText(TX::DebugRouteDiscovery)));
                break;
            }
        }
        msg = (const char*) "";
    }
        break;
    case loopRD: {
        // ToDo(AME): Repeat route discovery here if necessary to get better results
        uint8_t discoveredPeers =
                ModemInterpreter::getDiscManagerPtr()->getData("Num-LQI");
        if ((discoveredPeers == 1) || discoveredPeers < numMinPeers) {
            ProgramConfig::getTimeOutWatcherPtr()->clearTimeout(
                ProgramConfig::outerAssignmentFlowTOutID);
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::dTxtAutomatic014), discoveredPeers);
            msg = Global2::OutBuff;
            termFilterPtr_->println(TX::dTxtAutomatic015, TermContextFilter::filterDevelope);
            setFlowSteps(idle, idleRD, const_cast<char*>(TX::getText(TX::DebugRouteDiscovery)));
        } else {
            incFlowStep();
        }
        setAutomaticState(eStatus::remoteDiscovered, eStatusUpdate::set);
    }
        break;
    }

    return msg;
}

void AssignmentFlow::createRDTimeoutHandling() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->clearTimeout(ProgramConfig::assignmentFlowTOutID);
    termFilterPtr_->println(TX::dTxtAutomatic016, TermContextFilter::filterDevelope);

    pW->clearTimeout(ProgramConfig::routeDiscoveryTOutID);
    ITimeOutWatcher::timeOutInfo toi;
    pW->createDefaultTimeOut(&toi, ProgramConfig::routeDiscoveryTOutID,
                             &peerRouteDiscoveryTimeout, &peerRouteDiscoveryCallout);
    toi.delay = 600;            // 600 * 0.05s = 30s
    toi.timeCounter = 600;
    pW->registerTimeout(toi);
    termFilterPtr_->println(TX::dTxtAutomatic017, TermContextFilter::filterDevelope);
}

void AssignmentFlow::peerRouteDiscoveryTimeout() {
    termFilterPtr_->println(TX::dTxtAutomatic018, TermContextFilter::filterDevelope);
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->resetDelayForIndex(ProgramConfig::routeDiscoveryTOutID);
    // termDrivePtr_->println(sendDiscoveryRequest2Peer(targetIP, defaultSearchTime));
    if (flowStep == routeDiscovery) {
        ModemInterpreter::getDiscManagerPtr()->showData("T");
            // sendDiscoveryRequest2Peer(targetIP, defaultSearchTime);
    }
}

void AssignmentFlow::peerRouteDiscoveryCallout() {
    termFilterPtr_->println(TX::dTxtAutomatic019, TermContextFilter::filterDevelope);
    if (flowStep == routeDiscovery) {
        ModemInterpreter::getDiscManagerPtr()->showData("N");  // nextAutoDiscovery(targetIP);
    }
}

const char* AssignmentFlow::doBuildTree() {
    const char *msg = (const char*) "";
    bool done;
    int i;

    switch (flowSubStep) {
    default:
    case idleTree:
        break;
    case startTree:
        if (!termFilterPtr_->isTransmissionComplete()) {
            break;
        }
        termFilterPtr_->println(TX::AdjacencyMatrixFilled);
        adjazenzParentPtr_->fillMatrixFromDiscoveryData(false);
        setFlowSubStep(showTreeStart, const_cast<char*>(TX::getText(TX::DebugBuildTree)));
        break;
    case showTreeStart:
        adjazenzParentPtr_->showMatrixOrigin(Global2::OutBuff, Global2::outBuffMaxLength);
        termFilterPtr_->println(Global2::OutBuff, TermContextFilter::filterRoutingInfo);
        setFlowSubStep(showTreeNext, const_cast<char*>(TX::getText(TX::DebugBuildTree)));
        break;
    case showTreeNext:
        if (!termFilterPtr_->isTransmissionComplete()) {
            break;
        }
        i = 0;
        while (i < 10) {
            adjazenzParentPtr_->showGraphLine(Global2::OutBuff, Global2::outBuffMaxLength, &done);
            if (Global2::OutBuff[0]) {
                i++;
                termFilterPtr_->println(Global2::OutBuff, TermContextFilter::filterRoutingInfo);
            }
            if (done) {
                setFlowSubStep(showRouteStart, const_cast<char*>(TX::getText(TX::DebugBuildTree)));
                break;
            }
        }
        break;
    case showRouteStart:
        if (!termFilterPtr_->isTransmissionComplete()) {
            break;
        }
        i = 0;
        while (i < 10) {
            adjazenzParentPtr_->showRouteLine(Global2::OutBuff, Global2::outBuffMaxLength, &done);
            if (done) {
                setFlowSubStep(shortenTree, const_cast<char*>(TX::getText(TX::DebugBuildTree)));
                break;
            } else {
                if (Global2::OutBuff[0]) {
                    i++;
                    termFilterPtr_->println(Global2::OutBuff, TermContextFilter::filterRoutingInfo);
                }
            }
        }
        break;
    case shortenTree:
        adjazenzParentPtr_->shortenRouteList();
        termFilterPtr_->println(TX::dTxtAutomatic021);
        setFlowSubStep(showShortenedTree, const_cast<char*>(TX::getText(TX::DebugBuildTree)));
        break;
    case showShortenedTree:
        if (!termFilterPtr_->isTransmissionComplete()) {
            break;
        }
        i = 0;
        while (i < 10) {
            adjazenzParentPtr_->showRouteLine(Global2::OutBuff, Global2::outBuffMaxLength, &done);
            if (done) {
                setFlowSubStep(finishedTree, const_cast<char*>(TX::getText(TX::DebugBuildTree)));
                break;
            } else {
                if (Global2::OutBuff[0]) {
                    i++;
                    termFilterPtr_->println(Global2::OutBuff, TermContextFilter::filterRoutingInfo);
                }
            }
        }
        break;
    case finishedTree:
        flowSubStep = idleTree;
        if (!adjazenzParentPtr_->getRouteFailed()) {
            incFlowStep();
        }
        setAutomaticState(eStatus::treeCreated, eStatusUpdate::set);
        break;
    }
    return msg;
}

const char* AssignmentFlow::doProofTree() {
    const char *msg = (const char*) "";
    uint16_t routeEntrieAmount = 0;
    uint16_t connectedPeers = 0;
    static bool isRouteBroken = false;
    switch (flowSubStep) {
    case idleTProof:
        break;
    case treeProof: {
        routeEntrieAmount = adjazenzParentPtr_->getRouteListEntries(true);
        connectedPeers = ModemController::countActivNetworkMembers();
        isRouteBroken = (routeEntrieAmount != connectedPeers);
        setFlowSubStep(treeProofDecision, const_cast<char*>(TX::getText(TX::DebugProofTree)));
    }
        break;
    case treeProofDecision:
        if (isRouteBroken) {
            if (automaticMode) {
                setFlowSteps(routeDiscovery, startRemoteDiscovery, const_cast<char*>(TX::getText(TX::DebugProofTree)));
                msg = TX::getText(TX::MSTCleavedRestartSearch);
            } else {
                flowSubStep = idleTProof;
                msg = TX::getText(TX::MSTCleavedAutomaticStopped);
                clearAllFlowTimeouts();
            }
        } else {
            flowSubStep = idleTProof;
            incFlowStep();
            msg = TX::getText(TX::MSTAlright);
        }
        break;
    }
    return msg;
}

const char* AssignmentFlow::doAssignAutomatic(enum eCallCause cause) {
    const char *msg = (const char*) "";

    switch (flowSubStep) {
    default:
    case idleAssign:
        break;
    case startAssign:
        termFilterPtr_->println(TX::dTxtAutomatic022);
        msg = doStartAssign();
        break;
    case loopAssign:
        if (cause == plcMsg) {
            ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
            pW->resetDelayForIndex(ProgramConfig::assignmentFlowTOutID);
            pW->start(ProgramConfig::assignmentFlowTOutID);
            pW->resetDelayForIndex(ProgramConfig::outerAssignmentFlowTOutID);
            if (doAssignment(edaNext)) {
                break;
            }
            if (!processing) {
                pW->clearTimeout(ProgramConfig::assignmentFlowTOutID);
                uint8_t numConnected =
                        (uint8_t) ModemController::getNumPeersAliveAssigned();
                if (numConnected < numMinPeers) {
                    char cTemp[32];
                    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                            TX::getText(TX::dTxtAutomatic023));
                    snprintf(cTemp, sizeof(cTemp),
                            TX::getText(TX::dTxtAutomatic024));
                    strncat(Global2::OutBuff, cTemp, sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
                    msg = Global2::OutBuff;
                    setFlowSteps(idle, idleRouting, const_cast<char*>(TX::getText(TX::DebugDoAssign)));
                    clearAllFlowTimeouts();
                } else {
                    if (numConnected != cmpAssignedPeers) {
                        // ITimeOutWatcher::timeOutInfo *toi =
                        //        pW->findTimeOutWithID(ProgramConfig::outerAssignmentFlowTOutID);
                        if (assignLoopCallOuts > 1) {  // (toi->callOutCounter>1)
                            assignLoopCallOuts--;
                            setFlowSteps(buildTree, startTree, const_cast<char*>(TX::getText(TX::DebugDoAssign)));
                            msg = TX::getText(TX::dTxtAutomatic025);
                        }
                    } else {
                        if (automaticMode) {
                            incFlowStep();
                        } else {
                            termFilterPtr_->println(TX::StartingOfTripsPossible);
                            flowStep = idle;
                        }
                    }
                }
                setAutomaticState(eStatus::routeAssigned, eStatusUpdate::set);
            }
        }
        break;
    case nextAssign:
        setFlowSubStep(loopAssign, const_cast<char*>(TX::getText(TX::DebugDoAssign)));
        doAssignAutomatic(plcMsg);  // doAssignment(edaNext);
        break;
    }
    return msg;
}

const char* AssignmentFlow::doStartAssign() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->clearTimeout(ProgramConfig::assignmentFlowTOutID);
    pW->resetDelayForIndex(ProgramConfig::outerAssignmentFlowTOutID);
    ITimeOutWatcher::timeOutInfo toi;
    ITimeOutWatcher::createDefaultTimeOut(&toi,
                                          ProgramConfig::assignmentFlowTOutID, &assignmentTimeout,
                                          &assignmentCallOut);
//  toi.timeOutAmount   = 1;
//  toi.timeOutCounter  = 1;
//  toi.delay = 800;            // 800 * 0.05s = 40s
//  toi.timeCounter = 800;
    toi.callOutCounter = 1;
    pW->registerTimeout(toi);

    cmpAssignedPeers = (uint8_t) ModemController::getNumPeersConnected(false);
    return remoteBroadcastGroupSelection(const_cast<char*>("<?> all"));
}

void AssignmentFlow::assignmentTimeout() {
    termFilterPtr_->print(TX::DebugAssignmentTimeout);
    if (flowStep == sendBCOrder) {
        doAssignment(edaRepeat);
    }
}
void AssignmentFlow::assignmentCallOut() {
    if (flowStep == sendBCOrder) {
        setFlowSubStep(nextAssign, const_cast<char*>(TX::getText(TX::DebugAssignmentCallout)));
        ModemController::setAliveMemberByIdx(assignPeerIdx, false);
    }
}

void AssignmentFlow::handlePeerAssigned(uint16_t peerIP) {
    ModemController::setAliveMemberByIdx(peerIP, true);
}

void AssignmentFlow::printEOBBypassingResults() {
    for (int i = 0; i < 10; i++) {
        if (routeBypassInfo[i].receiverGroup != 0) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::ShowBypassGroupResults),
                    routeBypassInfo[i].receiverIP,
                    routeBypassInfo[i].receiverGroup,
                    routeBypassInfo[i].transmitterGroup);
        }
    }
}

const char* AssignmentFlow::doStartAlive() {
    const char *msg = (const char*) "";
    ITimeOutWatcher::timeOutInfo *pToi;

    switch (flowSubStep) {
    default:
    case idleAlive:
        break;
    case startAliveSub:
        ProgramConfig::getTimeOutWatcherPtr()->clearTimeout(
            ProgramConfig::outerAssignmentFlowTOutID);

        setFlowSubStep(waitAlive, const_cast<char*>(TX::getText(TX::DebugStartAlive)));
        ITimeOutWatcher::timeOutInfo toi;
        ITimeOutWatcher::createDefaultTimeOut(&toi,
                                              ProgramConfig::assignmentFlowTOutID,
                                              NULL, NULL);
        toi.timeOutAmount = 1;
        toi.timeOutCounter = 1;
        toi.callOutCounter = 1;
        toi.delay = 40;         // 20 * 0.05s = 2s
        toi.timeCounter = 40;
        ProgramConfig::getTimeOutWatcherPtr()->registerTimeout(toi);

        {  // Reduce output on the terminal, as output sometimes blocks interpretation of input
            uint32_t maske = TermContextFilter::getFilterMask();
            uint32_t neu = maske & ~TermContextFilter::filterPLCData;
            if (maske != neu) {
                TermContextFilter::setFilterMask((TermContextFilter::filterBits) neu);
                termFilterPtr_->println(TX::getText(TX::dTxtAutomatic027));
            }
        }

        termFilterPtr_->print(TX::dTxtAutomatic028);
        showCoordinatorVerboseBarExpectedTime(8);
        break;
    case waitAlive:
        pToi = ProgramConfig::getTimeOutWatcherPtr()->findTimeOutWithID(
            ProgramConfig::assignmentFlowTOutID);
        if (pToi) {
            if (pToi->started == false) {
                ProgramConfig::getTimeOutWatcherPtr()->clearTimeout(
                    ProgramConfig::assignmentFlowTOutID);
                setFlowSubStep(triggerAlive, const_cast<char*>(TX::getText(TX::DebugStartAlive)));
            }
        } else {
            setFlowSubStep(triggerAlive, const_cast<char*>(TX::getText(TX::DebugStartAlive)));
        }
        break;
    case triggerAlive: {
        termFilterPtr_->putchar(SerialStandardProfile::RETURN);
        // Start round trips, -1 = infinite
        bool success;
        msg = ForwardingFlow::callStartAliveRoundTrips(const_cast<char *>(" -1"),
                                                       true, &success);
        if (success) {
            ledFlowRef_.setLedMode(LEDSignalFlow::aliveRunning);
            setAutomaticState(eStatus::aliveStarted, eStatusUpdate::set);
        }
        setFlowSteps(idleAlive, idle, const_cast<char*>(TX::getText(TX::DebugStartAlive)));
        break;
    }
    }

    return msg;
}

void AssignmentFlow::showCoordinatorVerboseBarExpectedTime(
        uint8_t expectedTime) {
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
             "\r|%.*s|\r ", expectedTime, "-----------------------");
    termFilterPtr_->print(Global2::OutBuff, TermContextFilter::filterInitLoadBar);
}

uint16_t AssignmentFlow::getNextRouteIpAfterIp(uint16_t srcIP) {
    AdjazenzParent::tuple *routeEdgePtr = adjazenzParentPtr_->getRoute();
    for (int i = 0; i < kRouteListSize; i++) {
        if (routeEdgePtr[i].source == srcIP) {
            return routeEdgePtr[i].destination;
        }
    }
    return 0;
}

uint16_t AssignmentFlow::getRoutingIndexForIp(uint16_t targetIp) {
    AdjazenzParent::tuple *routeEdgePtr = adjazenzParentPtr_->getRoute();
    for (int i = 0; i < kRouteListSize; i++) {
        if (routeEdgePtr[i].source == targetIp) {
            return (uint16_t) i;
        }
    }
    return 0;
}

uint16_t AssignmentFlow::getRoutingIPForIndex(uint16_t index) {
    AdjazenzParent::tuple *routeEdgePtr = adjazenzParentPtr_->getRoute();
    return routeEdgePtr[index].source;
}

bool AssignmentFlow::isLastRoutingIP(uint16_t srcIP) {
    AdjazenzParent::tuple *routeEdgePtr = adjazenzParentPtr_->getRoute();
    return routeEdgePtr[adjazenzParentPtr_->getRouteListEntries(true)].source
            == srcIP;
}

bool AssignmentFlow::isPreLastRoutingIP(uint16_t srcIP) {
    AdjazenzParent::tuple *routeEdgePtr = adjazenzParentPtr_->getRoute();
    return routeEdgePtr[adjazenzParentPtr_->getRouteListEntries(true) - 1].source
            == srcIP;
}

char* AssignmentFlow::showMatrixOnTerminal(__attribute__((unused)) char *dummy) {
    if (adjazenzParentPtr_) {
        adjazenzParentPtr_->showMatrixOnTerminal();
    }
    return const_cast<char*>("");
}

char* AssignmentFlow::presetAliveRoute(char *text) {
    unsigned int cnt, val1;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u", &val1);
    if (cnt < 1) {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    }
    if (!ProgramConfig::isCoordinator()) {
        return const_cast<char*>(TX::getText(TX::UseCommandInCoordinatorModeOnly));
    }
    if (adjazenzParentPtr_) {
        if (adjazenzParentPtr_->setRoute(txt)) {
            setFlowStep(buildTree, const_cast<char*>(TX::getText(TX::TerminalMsg)));
            setFlowSubStep(showRouteStart, const_cast<char*>(TX::getText(TX::TerminalMsg)));
            return const_cast<char*>(TX::getText(TX::RouteSetTerminal));
        }
    }

    return const_cast<char*>("?");
}

// Status Stuff - - - - - - - - - - - - - - - - - - - - - - - - - - -
void AssignmentFlow::setAutomaticState(eStatus neu, eStatusUpdate mode) {
    switch (mode) {
    case eStatusUpdate::set:
        automaticStatus |= (uint8_t) neu;
        break;
    case eStatusUpdate::clear:
        automaticStatus &= (uint8_t) ~neu;
        break;
    case eStatusUpdate::overwrite:
        automaticStatus = neu;
        break;
    }
}

bool AssignmentFlow::testAutomaticState(eStatus test) {
    return (automaticStatus & test) != 0;
}

// TestSuite Stuff - - - - - - - - - - - - - - - - - - - - - - - - - - -
// getData - function to retrieve data of AssignmentFlow as string output
// @param                    |       | additional          |
//        cWhat              | count | parameter(s)        | description
//        ------------------ | ----- | ------------------- | ---------------------------------------------
//        "numConnectedPeers"|   0   |                     | get number of peers which have successfully
//                           |       |                     | connected to the PLC network
//        "numMinPeers"      |   0   |                     | number of peers which are needed for the test
//        "status"           |   1   | "remoteDiscovered"  | check if the remote discovery was handled
//        "status"           |   1   | "mstBuilded"        | check if the minimum spanning tree is builded
//        "status"           |   1   | "routeAssigned"     | check if the alive route is assigned to the peers
//        "status"           |   1   | "aliveStarted"      | check if the alive round trips were started
//        "status"           |   1   | "isIdle"            | check if automatization is in idle state
//        "status"           |   1   | "dwellTimeCompleted"| check if the dwell time to allow peers joining is completed
//        cOutput                   - buffer to print retrieved data in
//        outputLen                 - size of cOutput
//        cFormat                   - argument to format output.
//                                    See printf formating expressions
//        count                     - number of additional parameters if needed
//        ...                       - additional text parameters if needed
// @return true  - success - retrieved data are in cOutput - buffer
//         false - failed
#pragma GCC diagnostic ignored "-Wstack-usage="
bool AssignmentFlow::getData(const char *cWhat, char *cOutput, size_t outputLen,
        const char *cFormat, int count, ...) {
    // #define defaultResult (const char*) "syntax error"
    if (cOutput == NULL) {
        return false;
    }

    va_list args;
    va_start(args, count);
    bool success = false;
    char temp[80];
    size_t tmpLen = 0;

    if (count > 0) {
        tmpLen = snprintf(temp, sizeof(temp), "%s", va_arg(args, const char*));
    }
    if (strstr(cWhat, "numConnectedPeers")) {
        tmpLen = snprintf(temp, sizeof(temp), cFormat,
                ModemController::getNumPeersConnected(false));
        success = tmpLen < outputLen;
    }
    if (strstr(cWhat, "numMinPeers")) {
        tmpLen = snprintf(temp, sizeof(temp), cFormat, numMinPeers);
        success = tmpLen < outputLen;
    }
    if (strstr(cWhat, "status") && count == 1) {
        if (strstr(temp, "dwellTimeCompleted")) {
            bool test = testAutomaticState(eStatus::dwellTimeCompleted);
            tmpLen = snprintf(temp, sizeof(temp), cFormat, (test) ? 1 : 0);
            success = tmpLen < outputLen;
        } else if (strstr(temp, "remoteDiscovered")) {
            bool test = testAutomaticState(eStatus::remoteDiscovered);
            tmpLen = snprintf(temp, sizeof(temp), cFormat, (test) ? 1 : 0);
            success = tmpLen < outputLen;
        } else if (strstr(temp, "mstBuilded")) {
            bool test = testAutomaticState(eStatus::treeCreated);
            tmpLen = snprintf(temp, sizeof(temp), cFormat, (test) ? 1 : 0);
            success = tmpLen < outputLen;
        } else if (strstr(temp, "routeAssigned")) {
            bool test = testAutomaticState(eStatus::routeAssigned);
            tmpLen = snprintf(temp, sizeof(temp), cFormat, (test) ? 1 : 0);
            success = tmpLen < outputLen;
        } else if (strstr(temp, "aliveStarted")) {
            bool test = testAutomaticState(eStatus::aliveStarted);
            tmpLen = snprintf(temp, sizeof(temp), cFormat, (test) ? 1 : 0);
            success = tmpLen < outputLen;
        } else if (strstr(temp, "isIdle")) {
            bool test = (flowStep == idle);
            tmpLen = snprintf(temp, sizeof(temp), cFormat, (test) ? 1 : 0);
            success = tmpLen < outputLen;
        }
    }
    va_end(args);
    if (success) {
        strncpy(cOutput, temp, tmpLen);
        if (tmpLen <= outputLen) {
            cOutput[tmpLen] = '\0';
        }
        return true;
    } else {
        return false;
    }
}

// setData - function to set data of AssignmentFlow
// @param               |       | additional     |        |
//        cWhat         | count | parameter(s)   | range  | description
//        ------------- | ----- | -------------- | ------ | ---------------------------------------------
//        "numMinPeers" |   1   | decimal value  | 1..255 | number of peers which are needed for the test
// @return true  - success, data are set
//         false - unknown request or additional parameters are not in expected range
bool AssignmentFlow::setData(const char *cWhat, int count, ...) {
    bool success = false;
    va_list args;
    va_start(args, count);
    if (strstr(cWhat, "numMinPeers") && count == 1) {
        int tmp = va_arg(args, int);
        if (tmp > 0 && tmp < 256) {
            numMinPeers = (uint8_t) tmp;
            success = true;
        }
    }
    va_end(args);
    return success;
}

// runFunction - function to run within the AssignmentFlow
// @param
//        cWhat             | description
//        ----------------- | ---------------------------------------------
//        "all"             | start all steps of the automatization
//        "remoteDiscovery" | discover signal qualities of the peers
//        "buildTree"       | build minimum spanning tree
//        "assignRoute"     | send routing information to the peers
// @return true  - success, function started
//         false - unknown request
bool AssignmentFlow::runFunction(const char *cWhat) {
    bool success = false;

    if (strstr(cWhat, "all")) {
        setAutomaticMode(true);
        setFlowSteps(waitAutomaticA, idle, const_cast<char*>(TX::getText(TX::DebugUserInterface)));
        success = true;
    }

    if (strstr(cWhat, "remoteDiscovery")) {
        setFlowSteps(routeDiscovery, startRemoteDiscovery, const_cast<char*>(TX::getText(TX::DebugUserInterface)));
        success = true;
    }
    if (strstr(cWhat, "buildTree")) {
        setFlowSteps(buildTree, startTree, const_cast<char*>(TX::getText(TX::DebugUserInterface)));
        success = true;
    }
    if (strstr(cWhat, "assignRoute")) {
        setFlowSteps(sendBCOrder, startAssign, const_cast<char*>(TX::getText(TX::DebugUserInterface)));
        success = true;
    }

    return success;
}

AssignmentFlow::TupelContext* AssignmentFlow::getTupelContext(
        uint32_t conTextIdx) {
    const uint32_t sizeCreate = sizeof(IRoutingManager::MemberIpTupel)
            * maxTupelQuantity;
    const uint32_t sizeTranser = sizeof(IRoutingManager::MemberIpTupel)
            * IRoutingManager::maxTupelAmount;
    uint32_t size;
    TupelContext &p = tmpTupelContext[conTextIdx];
    if (conTextIdx < maxContexts) {
        if (p.pTupelArray == NULL) {
            size = (conTextIdx == create) ? sizeCreate : sizeTranser;
            p.pTupelArray = (IRoutingManager::MemberIpTupel*) malloc(size);
        }
        return &tmpTupelContext[conTextIdx];
    } else {
        return NULL;
    }
}

}  // namespace AME_SRC
