/*
 * SimpleDiscoveryManager.cpp
 *
 *  Created on: 25.05.2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "SimpleDiscoveryManager.h"

#include <cstdlib>

#include "../../G3PlcSystem/RequestFactory/RequestFactory.h"
#include "../../../HelpStructures/CharOperations.h"
#include "AssignmentFlow.h"

namespace AME_SRC {

SimpleDiscoveryManager::PeerSetLQI SimpleDiscoveryManager::peerArrayLQI[peerArrLQISize];  // = {}
uint8_t SimpleDiscoveryManager::peerArrayEntriesUsed = 0;
TermContextFilter *SimpleDiscoveryManager::termFilterPtr_ = NULL;

SimpleDiscoveryManager::SimpleDiscoveryManager(SerialDriver *termDrivePtr) :
        displayState(idle), displayEntryN(-1), displayActIdx(0),
        displayPeer(0), discoveryState(idleDiscovery), actionState(idleAction), targetIP(0),
        ignorePeerRxTimes(0), ignorePeerTxTimes(0), coordPeerAckReceived(ackUnknown),
        actCallNum(0), timeCounter(0) {
    termFilterPtr_ = new TermContextFilter(termDrivePtr);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    memset(peerArrayLQI, 0, sizeof(peerArrayLQI));  // Less code in compare with {} (look ahead)
    discoveryPendingMask.clear();
    numDiscoveriesToGo = 0;
    numDiscoveriesDone = 0;
}

// Query results from this class
// @param cWhat
//  - "LQI-Data <scrIP> <dstIP> Query discovery results of a peer.
//      - srcIp - container which at best contains an entry for dstIp
//      - dstIp - IP address of a peers which should be within a srcIp container
//      @return - Link quality entry of dstIp
// - "Num-LQI" - Query for the number of containers
//      @return - number of registered containers with link qualities including the container of the coordinator.
// - "status" - Query for the state of the class
//      @return - actual state with an element of eDiscoveryState
uint8_t SimpleDiscoveryManager::getData(const char *cWhat) {
    if (strstr(cWhat, "LQI-Data")) {
        const char *txt = charOperations::skip_token(cWhat, ' ');
        unsigned int dstIp, srcIp;
        int count = sscanf(txt, "%u %u", &dstIp, &srcIp);
        if (count == 2) {
            PeerSetLQI *pContainer = searchDiscoveryDataContainer((uint16_t) srcIp, false);
            if (pContainer != NULL) {
                for (uint32_t i = 0; i < pContainer->descriptorCounts; i++) {
                    if (pContainer->peerLqiArray[i].ipAdr == dstIp) {
                        return pContainer->peerLqiArray[i].linkQuality;
                    }
                }
            }
        }
        return 0;
    }
    if (strstr(cWhat, "Num-LQI")) {
        return peerArrayEntriesUsed;
    }
    if (strstr(cWhat, "status")) {
        return discoveryState;
    }
    return 0;
}

// Display the results of the peer discoveries
// The output is performed line by line
// @param cWhat - "A" - show all data
//        <PeerIP>- show data of peer with IP address PeerIP
//        <NULL>  - show next data set
char* SimpleDiscoveryManager::showData(const char *cWhat) {
    switch (*cWhat) {
        case 'T':   // timeout route discovery
        timeoutRDSend2Peer();
        break;
    default:
        wrapShowData(cWhat);
    }
    return const_cast<char*>("");
}

// helper for showData
void SimpleDiscoveryManager::wrapShowData(const char *cWhat) {
    bool lOutput = false;

    if (cWhat != NULL) {
        const char *test = strstr(cWhat, "A");
        if (test) {
            displayPeer = allPeers;
            displayState = start;
            displayActIdx = 0;
            displayEntryN = -1;
        } else {
            int peerIP;
            int count = sscanf(cWhat, "%04d", &peerIP);
            if ((count == 1) && (ModemController::getNetMemberPtrByIP((uint16_t) peerIP))) {
                displayPeer = (uint16_t) peerIP;
                displayState = start;
                displayActIdx = 0;
                displayEntryN = -1;
            } else {
                displayState = idle;
                return;
            }
        }
    }

    if (displayState == start) {
        displayState = pending;
        discoveryState = IDiscoveryManager::showDiscovery;
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::RDPeerEntries));
        lOutput = true;
        goto AnzeigeEnde;
    }

    if (displayState == pending) {
        if (displayActIdx >= peerArrayEntriesUsed) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::RDNoMoreEntries));
            displayState = idle;
            lOutput = true;
            goto AnzeigeEnde;
        }
        PeerSetLQI *p;
        if (displayPeer != allPeers) {
            p = searchDiscoveryDataContainer(displayPeer, false);
        } else {
            if (displayActIdx < peerArrayEntriesUsed && displayActIdx < peerArrLQISize) {
                p = &peerArrayLQI[displayActIdx];
            } else {
                displayState = idle;
                return;
            }
        }
        if (displayEntryN == -1) {
            displayEntryN++;
            if (p->peerIP != 0) {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowPeerWithIP), p->peerIP);
            } else {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowCoordinator));
            }
            strncat(Global2::OutBuff, TX::getText(TX::DescriptorIP_LQI), sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
            lOutput = true;
            goto AnzeigeEnde;
        }
        if (displayEntryN == 0 && p->descriptorCounts == 0) {
            showNextPeer();
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowPercent));
            lOutput = true;
            goto AnzeigeEnde;
        }
        if (displayEntryN < p->descriptorCounts) {
            uint32_t idx = displayEntryN++;
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::dTxtDM07), idx + 1,
                    p->peerLqiArray[idx].ipAdr,
                    p->peerLqiArray[idx].linkQuality);
            lOutput = true;
            goto AnzeigeEnde;
        }
        showNextPeer();
    } else {
        displayState = idle;
    }

AnzeigeEnde:
    if (lOutput && debugOutputEnabled()) {
        termFilterPtr_->println(Global2::OutBuff);
    }
    return;
}

// Select next peer
void SimpleDiscoveryManager::showNextPeer() {
    if (displayPeer != allPeers) {
        displayState = idle;
    }
    displayEntryN = -1;
    displayActIdx++;
}

bool SimpleDiscoveryManager::isDataShown() {
    return displayState == idle;
}

// Receive and manage data from ModemInterpreter
// @param status 'A' (Coor.)- Acknowledge of peer "Request to discovery command" received.
//               'M' (Peer, Coord.) - Data is a modem answer (PLC discovery command) with modem state = 0 -> no error
//               'm' (Peer, Coord.) - Data is  a modem answer (PLC discovery command) with modem state <> 0 -> error
//               'R' (Peer) - request to send a discovery command to peer modem, send acknowledge to coordinator.
//               'd' (Peer) - requests the last discovery results from peer.
//               'C' (coor.)- delete all entries of discovery results
//               'D' (coor.)- receive discovery data from peer - 1 or more entries are available in the list
//               'N' (Koor.)- no discovery data are available
void SimpleDiscoveryManager::storeData(uint8_t status, const uint8_t *data, __attribute__((unused)) uint8_t length) {
    switch (status) {
        case 'M':
            sortPeerLQI();
            break;
        case 'm': {
            uint16_t myIp = ModemController::getNetworkIPAddress();
            releasePeerLQI(myIp);
            break;
        }
    }

    if (ProgramConfig::isCoordinator()) {
        switch (status) {
        case 'A':
            coordPeerAckReceived = ackReceived;
            if (debugOutputEnabled()) {
                termFilterPtr_->println(TX::AcknowledgeReceived);
            }
            break;
        case 'C':
            clearAllDiscoveryData();
            break;
        case 'D':
        case 'N':
            registerDiscoveryData(data);
            break;
        case 'M':
            unSetPendingDiscovery(0);
            nextAutoDiscovery(targetIP, true);
            break;
        }
    } else {
        switch (status) {
        // case 'm': obsolete - negative Confirmation is send after a timeout
        case 'd':
            if (checkSendDiscoveryData(data)) {
                triggerSendDiscoveryData(true);
            }
            break;
        case 'M':
            triggerSendDiscoveryData(true);
            break;
        case 'R':
            triggerPeerRemoteDiscoveryRequest(data);
            break;
        }
    }
}

/***
Method to store discovery results of a peer at coordinator side (Remote Discovery)
***/
void SimpleDiscoveryManager::registerDiscoveryData(const uint8_t *data) {
    const uint8_t *messagePtr = data;
    uint16_t sourceIP = 0xFFFF;
    uint16_t destIP;
    PeerSetLQI *pContainer;
    NetworkMember *pNm;
    messagePtr = (const uint8_t*) strstr((const char*) messagePtr, ModemInterpreter::commandPrefix);
    // if (messagePtr==NULL)    // already checked in the calling routine
    messagePtr += 3;            // len(commandPrefix) + Letter SendRequest
    if (*messagePtr == 'D') {   // data
        messagePtr++;
        sourceIP = (uint16_t) atoi((const char*) (messagePtr));
        if (sourceIP) {
            pNm = ModemController::getNetMemberPtrByIP(sourceIP);  // findMemberWithShortID
            if (pNm == NULL) {
                return;
            }
        }
        uint16_t descriptorCounts;
        uint32_t j;
        messagePtr += 5;  // len(Ip-Str)
        pContainer = searchDiscoveryDataContainer(sourceIP, true);
        if (pContainer == NULL) {  // is there no empty container?
            return;
        }
        PeerLQI tempSet;
        pContainer->peerIP = sourceIP;
        descriptorCounts = *messagePtr++;
        for (j = 0; j < descriptorCounts && j < ModemController::maxPANDescriptorSets; j++) {
            destIP = *((uint16_t*) messagePtr);
                    // destIP = uint8Operations::sum2BytesFromLSB(messagePtr); BigEndian<->LittleEndian
            messagePtr += 2;
            pNm = ModemController::getNetMemberPtrByIP(destIP);
            if (pNm) {
                tempSet.ipAdr = destIP;
                tempSet.linkQuality = *messagePtr;
                peerLqiAdd(pContainer, tempSet);
            }
            messagePtr++;
        }
        unSetPendingDiscovery(sourceIP);
    } else if (*messagePtr == 'N') {  // negative acknowledge, no data
        sourceIP = (uint16_t) atoi((const char*) (messagePtr + 1));
        // releasePeerLQI(sourceIP); // do not remove an individual container
    }

    timeoutRestart();

    if (sourceIP != 0xFFFF) {
        nextAutoDiscovery(sourceIP, true);
    }
}

void SimpleDiscoveryManager::peerLqiAdd(PeerSetLQI *pContainer, const PeerLQI &newSet) {
    PeerLQI *pReadLqiContainer;
    uint16_t dcVorher = pContainer->descriptorCounts;
    bool add = true;
    for (uint32_t i = 0; i < dcVorher; i++) {
        pReadLqiContainer = &pContainer->peerLqiArray[i];
        if (pReadLqiContainer->ipAdr == newSet.ipAdr) {
            add = false;
            pReadLqiContainer->linkQuality = newSet.linkQuality;  // If necessary, carry out averaging
            break;
        }
    }
    if (add) {
        pReadLqiContainer = &pContainer->peerLqiArray[dcVorher++];
        pReadLqiContainer->ipAdr = newSet.ipAdr;
        pReadLqiContainer->linkQuality = newSet.linkQuality;
        pContainer->descriptorCounts = (uint8_t) dcVorher;
    }
}

void SimpleDiscoveryManager::clearAllDiscoveryData() {
    peerArrayEntriesUsed = ProgramConfig::isCoordinator() ? 0 : 1;
}


/***
 * short: Pre-check to query results from the peer
 * @param data pointer to the receiving string
 * @return true: the requested ip address matches with the address of the peer.
 */
bool SimpleDiscoveryManager::checkSendDiscoveryData(const uint8_t *data) {
    const uint8_t *messagePtr = data;
    uint16_t sourceIP = 0xFFFF;
    messagePtr = (const uint8_t*) strstr((const char*) messagePtr, ModemInterpreter::commandPrefix);
    // if (messagePtr==NULL)    // already checked in the calling routine
    messagePtr += 3;            // len(commandPrefix) + Letter SendRequest
    if (*messagePtr == 'd') {   // Request of data
        messagePtr++;
        sourceIP = (uint16_t) atoi((const char*) (messagePtr));
        if (sourceIP) {
            NetworkMember *pNm = ModemController::getNetMemberPtrByIP(sourceIP);  // findMemberWithShortID
            if (pNm != NULL) {
                return true;
            }
        }
    }
    return false;
}


/***
 * Short: send message to coordinator with discovery results from peer
 * @param noErrors - true: the Modem has transmitted the message without errors.
 */
void SimpleDiscoveryManager::triggerSendDiscoveryData(bool noErrors) {
    uint32_t nodeIdx = ModemController::getNextNodeStoreArrayIdx();
    PeerLQI peerLqiContainer;
    const uint32_t kSize = ModemController::maxPANDescriptorSets * size_eff_peerLQI + 1;
    uint8_t stream[kSize];
    // #if s_size >= 0x100  // DS: this request is not possible?
    //  #error "Stack error possible"
    // #endif
    uint8_t *pd;
    uint32_t dataLength;
    uint8_t foundDescriptorAmount;
    if (ignorePeerTxTimes > 0) {
        ignorePeerTxTimes--;
        termFilterPtr_->println(TX::RDTransmissionIgnoreOnce);
        return;
    }
    foundDescriptorAmount = ModemController::getFoundDescriptorAmount();
    if (noErrors && foundDescriptorAmount != 0) {
        PeerSetLQI &pOuter = peerArrayLQI[0];       // container of a peer
        PeerLQI *pInner = &pOuter.peerLqiArray[0];
        pd = stream;
        *pd++ = foundDescriptorAmount;      // number of elements
        for (uint32_t i = 0; i < ModemController::maxPANDescriptorSets
                && i < foundDescriptorAmount; i++) {
            peerLqiContainer.ipAdr = pInner->ipAdr;
            peerLqiContainer.linkQuality = pInner->linkQuality;
            memcpy(pd, (const void*) &peerLqiContainer, size_eff_peerLQI);
            pd += size_eff_peerLQI;
            pInner++;
        }
        dataLength = pd - stream;
        ModemController::storeDataAtNodeIndex(nodeIdx, 'D', stream,
                (uint8_t) dataLength);  // 'D' = Data
    } else {
        ModemController::storeDataAtNodeIndex(nodeIdx, 'N', NULL, 0);  // negative confirmation
    }

    List<PlcCommand> &getRemoteDiscoveryList = ModemController::getNodeCmdDefaultList();
    getRemoteDiscoveryList.clear();
    getRemoteDiscoveryList.add(ModemController::getNodeStoreArrayPtrByIdx(nodeIdx));
    ModemController::setControllerState(ModemController::RemoteDiscoveryConfirm);
    // ModemInterpreter::setRemoteNetworkSearch(false);
    ModemInterpreter::setInterpretState(ModemInterpreter::idle);

    ModemController::setProcessState(getRemoteDiscoveryList.getHead());
}


/***
Find a suitable container for the peer's discovery results.
If necessary, a new, free container will be returned.*
* @param ipAdr          IP address of the peer, which has to be searched in the array
* @param newContainer   true: a new container may be returned. false: don't use a new container, return = NULL
***/
SimpleDiscoveryManager::PeerSetLQI* SimpleDiscoveryManager::searchDiscoveryDataContainer(
        uint16_t ipAdr, bool newContainer) {
    PeerSetLQI *p;
    if (!ProgramConfig::isCoordinator()) {
        peerArrayEntriesUsed = 1;
        return &peerArrayLQI[0];
    }
    for (uint32_t i = 0; i < peerArrayEntriesUsed && i < peerArrLQISize; i++) {
        p = &peerArrayLQI[i];
        if (p->peerIP == ipAdr) {
            return p;
        }
    }
    if (newContainer && peerArrayEntriesUsed < peerArrLQISize) {
        p = &peerArrayLQI[peerArrayEntriesUsed++];
        p->descriptorCounts = 0;
        return p;
    }
    return NULL;
}

/***
Method of peer, which handles a remote discovery. The method is triggered by the coordinator.
An acknowledge is send to the coordinator.
***/
#pragma GCC diagnostic ignored "-Wstack-usage="
void SimpleDiscoveryManager::triggerPeerRemoteDiscoveryRequest(const uint8_t *data) {
    enum { ofsParameter = 4 };
    CHANNEL id = channel0;
    uint32_t nodeIdx = ModemController::getNextNodeStoreArrayIdx();
    const char *dta = strstr((const char*) data, ModemInterpreter::commandPrefix);
    unsigned int searchTime;
    int destIP;
    int cnt = sscanf(dta + ofsParameter, "%d %u", &destIP, &searchTime);
    if (cnt != 2 || ((destIP > 0) &&
       (destIP != ModemController::getNetMemberPtrByIndex(1)->getNetworkIPAddress()))) {
        return;
    }
    if (ignorePeerRxTimes) {  // TEST: option to ignore request from coordinator
        --ignorePeerRxTimes;
        termFilterPtr_->println(TX::RDRequestIgnoreOnce);
        return;
    }

    List<PlcCommand> &getRemoteDiscoveryList = ModemController::getNodeCmdDefaultList();
    getRemoteDiscoveryList.clear();

    ModemController::storeDataAtNodeIndex(nodeIdx, 'A', NULL, 0);  // Node n =>  PLC-Send Data (prefix=ACK)
    getRemoteDiscoveryList.add(ModemController::getNodeStoreArrayPtrByIdx(nodeIdx));

    nodeIdx = ModemController::getNextNodeStoreArrayIdx();
    Node<PlcCommand> &p = ModemController::getNodeStoreArrayPtrByIdx(nodeIdx);
    p = Node<PlcCommand>(&RequestFactory::createRequest(PLC_CommandTypeSpecification::searchNetwork, 2, id,
                                                        searchTime));
    getRemoteDiscoveryList.add(
            ModemController::getNodeStoreArrayPtrByIdx(nodeIdx));
    ModemController::setControllerState(
            ModemController::RemoteDiscoveryRequest);
    // ModemInterpreter::setRemoteNetworkSearch(true);         // DS, ToDo(AME): check if obsolete
    // ModemInterpreter::setInterpretState(ModemInterpreter::proofDiscoveryTimeout);    // DS, 05.12.2023
    actionState = waitPeerDiscovery;
    timeoutRestart();
    ModemController::setProcessState(getRemoteDiscoveryList.getHead());
}

// Peer: move and sort Pan Descriptor entries into the PeerLQI structure
void SimpleDiscoveryManager::sortPeerLQI() {
    uint16_t lqi, found_lqi, max_lqi;
    uint32_t ref_idx;
    uint8_t foundDescriptorAmount = ModemController::getFoundDescriptorAmount();
    BitMask<uint32_t> mask;  // up to 32 elements: foundDescriptorAmount<32 !
                             // Will be uses for link qualities with same value
    uint16_t containerIP = ModemController::getNetworkIPAddress();
    if (!ProgramConfig::isCoordinator() && !ModemController::isNetworkMemberIPAktiv(0)) {
        return;     // peer not yet connected Peer. => IP address undefined
    }
    PeerSetLQI *pOuter = searchDiscoveryDataContainer(containerIP, true);
    if (pOuter == NULL) {
        return;
    }
    PeerLQI tempSet;
    pOuter->peerIP = containerIP;

    max_lqi = 0xFFFF;
    for (uint32_t j = 0; j < ModemController::maxPANDescriptorSets && j < foundDescriptorAmount; j++) {
        found_lqi = 0;
        ref_idx = 0xFFFFFFFF;
        // Search for Pan-Descriptor with the highest link quality
        for (uint32_t i = 0; i < ModemController::maxPANDescriptorSets && i < foundDescriptorAmount; i++) {
            ModemController::PANDescriptor &ps = ModemController::getPanDescriptorContainerPtrByIdx(i);
            lqi = uint8Operations::sum2BytesFromLSB((const uint8_t*) &ps.linkQuality);
            if ((lqi >= found_lqi) && (lqi <= max_lqi) && !mask.isSet(i)) {
                found_lqi = lqi;
                ref_idx = i;
            }
        }
        // store result and select next container
        if (ref_idx != 0xFFFFFFFF) {
            ModemController::PANDescriptor &ps = ModemController::getPanDescriptorContainerPtrByIdx(ref_idx);
            tempSet.ipAdr = ps.adress;
            tempSet.linkQuality = (uint8_t) found_lqi;
            peerLqiAdd(pOuter, tempSet);
            max_lqi = found_lqi;
            mask.set(ref_idx);
        } else {
            break;
        }
    }
}

// Delete container for Peer LQI values if there are no PANDescriptor available entries
void SimpleDiscoveryManager::releasePeerLQI(uint16_t containerIP) {
    PeerSetLQI *pContainer = searchDiscoveryDataContainer(containerIP, true);
    if (pContainer == NULL) {  // no empty container for ip address found?
        return;
    }
    pContainer->peerIP = containerIP;
    pContainer->descriptorCounts = 0;
}

// Interface to trigger the discovery
// (Will only used at coordinator)
// @param status 'T' - Remote Discovery for peer N via Terminal
//               'S' - Remote Discovery for all peers
//               '5' - time controlled actions from 50ms task (coordinator, peer)
char* SimpleDiscoveryManager::triggerDiscovery(uint8_t select, const char *cParameter) {
    switch (select) {
        case 'T':  // Terminal
            actionState = manual;
            return terminalNetworkSearch(cParameter);
        case 'S':  // start discovery automatic
            actionState = automatic;
            startAutoDiscovery(false);
            break;
        case '5':
            return _50msActions();
    }
    return const_cast<char*>("");
}

// text:
// "RNetworkSearch S" @ coordinator: determine all route results
// "RNetworkSearch <x> <y>" determine route at peer x for y seconds
// "RNetworkSearch I<x> @Peer: Remote-Discovery requests from coordinator are ignored <x> times
// "RNetworkSearch i<x> @Peer: Remote-Discovery sending of results to coordinator will be suppressed <x> times
char* SimpleDiscoveryManager::terminalNetworkSearch(const char *text) {
    unsigned int peerIP, searchTime, nTimesIgnore;
    int count;
    const char *txt = charOperations::skip_token(text, ' ');
    if (*txt == 'I') {  // Test @peer to ignore requests from coordinator
        count = sscanf(txt + 1, "%u", &nTimesIgnore);
        if (count != 1 || nTimesIgnore > 10) {
            nTimesIgnore = 1;
        }
        ignorePeerRxTimes = (uint8_t) nTimesIgnore;
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::RDRequestWillBeIgnored), nTimesIgnore);
        return const_cast<char*>(Global2::OutBuff);
    }
    if (*txt == 'i') {  // Test @peer to ignore sending while receiving coordinator requests
        count = sscanf(txt + 1, "%u", &nTimesIgnore);
        if (count != 1 || nTimesIgnore > 10) {
            nTimesIgnore = 1;
        }
        ignorePeerTxTimes = (uint8_t) nTimesIgnore;
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::RDTxSuppressedNTimes), nTimesIgnore);
        return const_cast<char*>(Global2::OutBuff);
    }
    count = sscanf(txt, TX::getText(TX::dTxtDM08), &peerIP, &searchTime);  // "%u %u"
    if (count != 2) {
        return const_cast<char*>(TX::getText(TX::dTxtInvalidArguments));
    }

    char *errorMessage = getRSearchErrorMessage((uint16_t) peerIP, (uint8_t) searchTime);
    if (errorMessage == NULL) {
        discoveryState = IDiscoveryManager::discovery;
        if (debugOutputEnabled()) {
            termFilterPtr_->println(TX::RDSearchIsStarted);
        }
        char *msg = sendDiscoveryRequest2Peer((uint8_t) peerIP, (uint16_t) searchTime, requestStart);
        if (debugOutputEnabled()) {
            termFilterPtr_->print(msg);
        }
        AssignmentFlow::showCoordinatorVerboseBarExpectedTime((uint8_t) searchTime);
        return const_cast<char*>("");
    } else {
        return errorMessage;
    }
}

// Start Remote Discovery at all connected peers
void SimpleDiscoveryManager::startAutoDiscovery(bool lRestart) {
    if (!lRestart) {
        actCallNum = 0;
        clearAllDiscoveryData();
        discoveryState = IDiscoveryManager::discovery;
        discoveryPendingMask.clear();
        discoveryPendingMask.set(0);    // coordinator
        numDiscoveriesToGo = 1;
        numDiscoveriesDone = 0;
        for (int i = 1; i <= ModemController::getNetworkMemberMax(); i++) {
            NetworkMember *pn = ModemController::getNetMemberPtrByIndex((uint8_t) i);
            if (pn != NULL && pn->connectedOrNotVerified()) {
                discoveryPendingMask.set(i);
                numDiscoveriesToGo++;
            }
        }
    }
    if (discoveryPendingMask.isSet(0)) {
        if (debugOutputEnabled()) {
            termFilterPtr_->println(TX::RDSearchIsStarted);
        }
        char *msg = sendDiscoveryRequest2Peer(0, defaultSearchTime, requestStart);
        if (debugOutputEnabled()) {
            termFilterPtr_->print(msg);
        }
        AssignmentFlow::showCoordinatorVerboseBarExpectedTime(defaultSearchTime);
    } else {
        targetIP = 0;
        nextAutoDiscovery(0, false);
    }
}

void SimpleDiscoveryManager::nextAutoDiscovery(uint16_t parameter, bool ipOrIndex) {
    int runde;
    int index = -1;
    if (ipOrIndex) {  // true: parameter = IP
        if (parameter == targetIP) {
            index = ModemController::getNetMemberIdxByIP(parameter);
        }
    } else {
        index = parameter;
    }
    if (actionState == automatic) {
        if (debugOutputEnabled()) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::RDShowPendingMask),
                    (unsigned int) discoveryPendingMask.getAllMask(), index);
            termFilterPtr_->println(Global2::OutBuff);
        }
        runde = 1;
        while (runde <= 2) {
            if (index >= 0) {
                for (int i = index + 1;
                        i <= ModemController::getNetworkMemberMax(); i++) {
                    NetworkMember *pn = ModemController::getNetMemberPtrByIndex(
                            (uint8_t) i);
                    if (pn != NULL && pn->connectedOrNotVerified() && discoveryPendingMask.isSet(i)) {
                        actCallNum = 0;
                        char *msg = sendDiscoveryRequest2Peer((uint8_t) i, defaultSearchTime, requestStart);
                        if (debugOutputEnabled()) {
                            termFilterPtr_->print(msg);
                        }
                        AssignmentFlow::showCoordinatorVerboseBarExpectedTime(
                                defaultSearchTime);
                        return;
                    }
                }  // next
            }
            if (discoveryPendingMask.getAllMask() == 0) {
                break;
            }
            index = 0;
            runde++;
            if (debugOutputEnabled()) {
                termFilterPtr_->println(TX::RDNextRepeatOnce);
            }
        }
        if (checkDiscoveryResults()) {
            if (debugOutputEnabled()) {
                termFilterPtr_->println(TX::RDNextRepeatPeers);
            }
            timeoutRestart();
            return startAutoDiscovery(true);
        }
        ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
        pW->clearTimeout(ProgramConfig::routeDiscoveryTOutID);
        if (debugOutputEnabled()) {
            termFilterPtr_->println(TX::RDNextTimeoutCleared);
        }
        actionState = show;    // routingFinished = true;
        discoveryState = IDiscoveryManager::showDiscovery;
        // ModemInterpreter::setInterpretState(ModemInterpreter::idle);  // will be set in showData
        if (debugOutputEnabled()) {
            termFilterPtr_->println(TX::RDiscoveryFinished);
        }
    }
}

// Check the consistency of the root discovery results and repeat the determination for individual peers if necessary
// Check if:
// - Root Discovery results of individual peers are available
// - signal qualities of the individual results are different from zero
// Return:
//  false: no more discoveries to be considered
//   true: minimum one discovery has to be repeated
bool SimpleDiscoveryManager::checkDiscoveryResults() {
    if (++actCallNum >= defaultCallOuts) {
        return false;
    }

    PeerSetLQI *pLQI;
    numDiscoveriesToGo = 0;  // coordinator is excluded
    numDiscoveriesDone = 0;
    for (int i = 1; i <= ModemController::getNetworkMemberMax(); i++) {
        NetworkMember *pn = ModemController::getNetMemberPtrByIndex((uint16_t) i);
        if (pn != NULL && pn->connectedOrNotVerified()) {
            bool lSet = false;
            pLQI = searchDiscoveryDataContainer(pn->getNetworkIPAddress(), false);
            if (pLQI == NULL) {
                lSet = true;
            } else {
                if (pLQI->descriptorCounts == 0) {
                    lSet = true;
                } else {
                    for (int j = 0; j < pLQI->descriptorCounts; j++) {
                        if (pLQI->peerLqiArray[j].linkQuality == 0) {
                            pLQI->descriptorCounts = 0;
                            lSet = true;
                            break;
                        }
                    }
                }
            }
            if (lSet) {
                discoveryPendingMask.set(i);
                numDiscoveriesToGo++;
            }
        }
    }

    return numDiscoveriesToGo != 0;
}


// Register that discovery data has arrived from the peer,coordinator
// => management of the discoveryPendingMask
void SimpleDiscoveryManager::unSetPendingDiscovery(uint16_t peerIP) {
    int idx = ModemController::getNetMemberIdxByIP(peerIP);
    if ((idx >= 0) && (idx < 32)) {
        if (discoveryPendingMask.isSet(idx)) {
            discoveryPendingMask.unSet(idx);
            numDiscoveriesDone++;
        }
    }
}


void SimpleDiscoveryManager::timeoutRDSend2Peer() {
    sendDiscoveryRequest2Peer(targetIP, defaultSearchTime, requestStart);
}

char* SimpleDiscoveryManager::sendDiscoveryRequest2Peer(uint8_t peerIP,
                                                        uint16_t searchTime, eSendDiscoveryRequest2PeerMode mode) {
    char dump[24];
    targetIP = peerIP;
    if (peerIP) {
        if (mode == requestStart) {
            coordPeerAckReceived = (uint8_t) ackPending;
            // "%s%cR%03d %02d" option for parallel determination with -1 for peerIP
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtDM10),
                    ModemInterpreter::commandPrefix,
                    ModemInterpreter::SearchRequest, peerIP, searchTime);
        } else {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%cd%03d",
                    ModemInterpreter::commandPrefix,
                    ModemInterpreter::SearchRequest, peerIP);
        }
#ifdef ALIVE_PREFER_BROADCAST_TRANSMISSION
        ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                reinterpret_cast<uint8_t*>(Global2::OutBuff), (uint8_t) strlen(Global2::OutBuff),
                IRoutingManager::BCGlobalIP, true,
                TermContextFilter::filterDiscoveryInfo);
#else
    ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(Global2::OutBuff, peerIP, false);
#endif
        // ModemInterpreter::setInterpretState(ModemInterpreter::proofDiscoveryTimeout);  // DS, 05.12.2023
        // "Discovery @Peer %04d"
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtDM11), peerIP);
    } else {
        coordPeerAckReceived = (uint8_t) ackReceived;  // No acknowledgment is required from the coordinator
        ModemController::triggerSinglePlcRequestWithArgs(
                PLC_CommandTypeSpecification::searchNetwork, 2, channel0,
                searchTime);
        // "Discovery @coordinator"
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtDM12));
    }
    snprintf(dump, sizeof(dump), TX::getText(TX::ProgressXofY), numDiscoveriesDone + 1, numDiscoveriesToGo);
    strncat(Global2::OutBuff, dump, sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
    timeoutRestart();
    return Global2::OutBuff;
}

// Helper for terminalNetworkSearch
char* SimpleDiscoveryManager::getRSearchErrorMessage(uint16_t peerIP, uint8_t searchTime) {
    enum {
        searchMaxTime = 30
    };
    if (peerIP > ModemController::getNetworkMemberMax()) {
        return const_cast<char*>(TX::getText(TX::dTxtDM13));  // "IP zu hoch!"
    } else if (!ModemController::isNetworkMemberIPAktiv(peerIP)
            && (!ProgramConfig::isCoordinator() && peerIP != 0)) {
        return const_cast<char*>(TX::getText(TX::dTxtDM14));  // "Teilnehmer nicht active!";
    } else if (searchTime > searchMaxTime) {
        return const_cast<char*>(TX::getText(TX::dTxtDM15));  // "Suchzeit ungueltig!";
    } else {
        return NULL;
    }
}

char* SimpleDiscoveryManager::_50msActions() {
    const uint16_t timeOutAck = 100;    // 100 * 0.05s =  5s
    const uint16_t timeOutData = 500;   // 500 * 0.05s = 25s
    int index;
    char *toText;
    enum eSendDiscoveryRequest2PeerMode mode;

    switch (actionState) {
    case automatic:
        timeCounter++;
        if (coordPeerAckReceived == ackPending && timeCounter == timeOutAck) {
            toText = const_cast<char*>(TX::getText(TX::TimeoutAcknowledge));
            mode = requestStart;
        } else if (coordPeerAckReceived == ackReceived && timeCounter == timeOutData) {
            toText = const_cast<char*>(TX::getText(TX::TimeoutRemoteData));
            mode = requestData;
        } else {
            toText = NULL;
        }
        if (toText) {
            termFilterPtr_->println(toText);
            timeoutRestart();
            if (++actCallNum <= defaultCallOuts) {
                termFilterPtr_->print(sendDiscoveryRequest2Peer(targetIP, defaultSearchTime, mode));
            } else {
                index = ModemController::getNetMemberIdxByIP(targetIP);
                if (index > 0) {
                    ModemController::disableNetMemberAtIndex((uint16_t) index, true, true);
                }
                targetIP = (uint8_t) index;
                actionState = auto_next;
            }
        }
        break;
    case auto_next:
        actionState = automatic;
        nextAutoDiscovery((uint16_t) targetIP, false);  // targetIP = index
        break;
    case show:
        if (termFilterPtr_->isTransmissionComplete()) {
            actionState = idleAction;
            return showData("A");
        }
        break;
    case waitPeerDiscovery:
        if (timeCounter++ == timeOutData) {
            timeoutRestart();
            triggerSendDiscoveryData(false);
        }
        break;
    }
    return NULL;
}

void SimpleDiscoveryManager::timeoutRestart() {
    timeCounter = 0;
    bool output = true;
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    ITimeOutWatcher::timeOutInfo *infoPtr = pW->findTimeOutWithID(ProgramConfig::routeDiscoveryTOutID);
    if (infoPtr) {
        // was timeout already cleared?
        if (infoPtr->timeCounter == infoPtr->delay) {
            output = false;  // yes: suppress doubled outputs
        }
    }
    pW->resetDelayForIndex(ProgramConfig::routeDiscoveryTOutID);
    if (debugOutputEnabled() && output) {
        termFilterPtr_->println(TX::RDNextTimeoutRestarted);
    }
}

bool SimpleDiscoveryManager::debugOutputEnabled() {
    return termFilterPtr_->isNotFiltered((TermContextFilter::filterBits)
            (TermContextFilter::filterDevelope |
             TermContextFilter::filterRoutingInfo));
}

/***
void SimpleDiscoveryManager::createTimeoutHandling() {
    ITimeOutWatcher *pW =ProgramConfig::getTimeOutWatcherPtr();
    pW->clearTimeout(ProgramConfig::routeDiscoveryTOutID);
    ITimeOutWatcher::timeOutInfo toi;
    pW->createDefaultTimeOut(&toi,ProgramConfig::routeDiscoveryTOutID, &peerDiscoveryTimeout, &peerDiscoveryCallout);
    toi.delay = 600;            // 600 * 0.05s = 30s
    toi.timeCounter = 600;
    pW->registerTimeout(toi);
}

void SimpleDiscoveryManager::peerDiscoveryTimeout() {
    ITimeOutWatcher *pW =ProgramConfig::getTimeOutWatcherPtr();
    pW->restartTimeout(ProgramConfig::routeDiscoveryTOutID);
    // termFilterPtr_->println(sendDiscoveryRequest2Peer(targetIP, defaultSearchTime));
    sendDiscoveryRequest2Peer(targetIP, defaultSearchTime);
}

void SimpleDiscoveryManager::peerDiscoveryCallout() {
    // termFilterPtr_->println(nextAutoDiscovery(targetIP));
    nextAutoDiscovery(targetIP);
}
***/

SimpleDiscoveryManager::~SimpleDiscoveryManager() = default;

}  // namespace AME_SRC
