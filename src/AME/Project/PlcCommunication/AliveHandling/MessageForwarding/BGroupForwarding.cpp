/*
 * BGroupForwarding.cpp
 *
 *  Created on: Aug 4, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "BGroupForwarding.h"

#include <stdlib.h>

#include "../NeighbourAssignment/AssignmentFlow.h"
#include "../../../SystemBase/ProgramConfig.h"
#include "../../PlcTerminalCommandConnector.h"
#include "../../ModemInterpreter.h"
#include "../../../SystemLogic/StorageAdministrator.h"
#include "../../../SystemLogic/I18N.h"

namespace AME_SRC {
BGroupForwarding::aliveContainer BGroupForwarding::aliveMessageContainer = BGroupForwarding::aliveContainer();
BGroupForwarding::endTransmitterBypass BGroupForwarding::bypassInfo = {IRoutingManager::notUsedID, 0};
bool BGroupForwarding::enabled_ = false;
bool BGroupForwarding::isReversRouteAlerted_ = false;
bool BGroupForwarding::isReverseRoutingEnabled_ = false;
uint8_t BGroupForwarding::lastRoundTripId_ = 0;
AliveData* BGroupForwarding::outgoingAlive_ = NULL;
ITimeOutWatcher* BGroupForwarding::watcher_ = NULL;
SimpleTimeOutWatcher::timeOutInfo BGroupForwarding::noResponseReportTimeOut_;
SimpleTimeOutWatcher::timeOutInfo BGroupForwarding::repeatTimeOut_;
SimpleTimeOutWatcher::timeOutInfo BGroupForwarding::repeatAliveEndTimeout_;
TermContextFilter *BGroupForwarding::termFilterPtr_ = NULL;

#pragma GCC diagnostic ignored "-Wstack-usage="
BGroupForwarding::BGroupForwarding(SerialDriver *notifyRef, IRoutingManager &routingManager,
        ITimeOutWatcher &watcher, LEDSignalFlow &ledFlowRef) :
        routingManager_ { routingManager }, ledFlowRef_ { ledFlowRef }, indicationCount_ {
                0 }, defaultForwardGroup { 0xFFFF } {  // ,outgoingAlive_{0}
    termFilterPtr_ = new TermContextFilter(notifyRef);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);

    ITimeOutWatcher::createTripleTimeOut(&repeatTimeOut_,
                                         ProgramConfig::rAliveTOutID, &retransmitRequest, &handleAliveNoResponseTimeOut);
    repeatTimeOut_.delay = 20;
    repeatTimeOut_.timeCounter = 20;
    repeatTimeOut_.timeCounter = 20;
    repeatTimeOut_.timeOutCounter = 10;
    repeatTimeOut_.timeOutAmount = 10;

    // ToDo(AME): A scenario for the absence of the coordinator reaction still needs to be defined here
    ITimeOutWatcher::createDefaultTimeOut(&noResponseReportTimeOut_,
                                          ProgramConfig::aliveNoResponsReportTOutID, &transmitAlarmRequest, NULL);
    noResponseReportTimeOut_.delay = 20;
    noResponseReportTimeOut_.timeCounter = 20;
    noResponseReportTimeOut_.timeOutAmount = 70;
    noResponseReportTimeOut_.timeOutCounter = 70;
    noResponseReportTimeOut_.callOutCounter = 10;
    noResponseReportTimeOut_.callOutAmount = 10;
    watcher_ = &watcher;
    watcher_->registerTimeout(repeatTimeOut_);
    watcher_->registerTimeout(noResponseReportTimeOut_);
    watcher_->stopAndReset(ProgramConfig::aliveNoResponsReportTOutID);
    if (ProgramConfig::isCoordinator()) {
        ITimeOutWatcher::createTripleTimeOut(&repeatAliveEndTimeout_,
                                             ProgramConfig::aliveACKLastTripID, &retransmitLastAckRequest, &retransmitLastAckCallout);
        watcher_->registerTimeout(repeatAliveEndTimeout_);

        ITimeOutWatcher::timeOutInfo toi;
        toi.id = ProgramConfig::aliveACKReplyOnceID;
        toi.timeCounter = 40;       // 40 * 0.05s = 2000ms
        toi.delay = 40;
        toi.timeOutCounter = 0;
        toi.timeOutAmount = 0;
        toi.callOutCounter = 0;
        toi.callOutAmount = 0;
        toi.timeOutFunc = NULL;
        toi.callOutFunc = NULL;
        toi.started = false;
        watcher_->registerTimeout(toi);
    }
    outgoingAlive_ = new AliveData(AliveData::outgoing, IRoutingManager::BGRoundStart, IRoutingManager::Transmitter);
}

void BGroupForwarding::forward(AliveData &incomingAlive) {
    uint8_t hostID;     // => This is the roll number of the receiving peer
    uint8_t memberID;   // => This is the roll number of the peer which has the message sent

    hostID = getHostRoleIDForIncomingGroup(incomingAlive.getIncomingGroup());
    if (hostID == IRoutingManager::notUsedID) {
        // handleSuspendRetransmit(incomingAlive);
        return;
    }
    memberID = incomingAlive.getMemberID();

    AliveData::aliveType aliveType = classifyAliveWithIDs(hostID, memberID);
    if ((incomingAlive.getRoundTripID() != lastRoundTripId_) || (aliveType != AliveData::Alive_Request)) {
        incomingAlive.setStatus(aliveType);
        reactOnAliveWithHostID(incomingAlive);
    }
}


uint8_t BGroupForwarding::getHostRoleIDForIncomingGroup(uint16_t incomingGroup) {
    routingManager_.storeData(IRoutingManager::SEARCH_Value, incomingGroup);
    return (uint8_t) routingManager_.getData(IRoutingManager::GET_MEMBER_ID_AT_SEARCH_VAL);
}


AliveData::aliveType BGroupForwarding::classifyAliveWithIDs(uint8_t hostID, uint8_t memberID) {
    switch (hostID) {
        case IRoutingManager::Receiver:
            if (memberID == IRoutingManager::Transmitter
                    || memberID == IRoutingManager::EndTransmitter
                    || memberID == IRoutingManager::ListenerF1) {
                return AliveData::Alive_Request;
            }
        break;
        case IRoutingManager::Supervisor:
            if (memberID == IRoutingManager::Transmitter) {
                return AliveData::Alive_Confirm;
            }
            break;
        case IRoutingManager::Administrator:
            if (memberID == IRoutingManager::Transmitter) {
                return AliveData::Alive_RoundTrip;
            }
            break;
        case IRoutingManager::ListenerF1:
            if (memberID == IRoutingManager::Transmitter) {
                return AliveData::Alive_Indication;
            }
            break;
        case IRoutingManager::ListenerB1:
            if (memberID == IRoutingManager::Transmitter) {
                return AliveData::Alive_Confirm;
            }
            break;
        case IRoutingManager::Transmitter:
        default:
            break;
    }
    return AliveData::Alive_Unknown;
}


void BGroupForwarding::reactOnAliveWithHostID(AliveData &incomingAlive) {
    bool isCoordinator = ProgramConfig::isCoordinator();
    if (isCoordinator) {
        ledFlowRef_.setIsAliveSignaled(true);
    }
    switch (incomingAlive.getStatus()) {
        case AliveData::Alive_Request:
            forwardAliveMessage(incomingAlive);
            if (!isCoordinator) {
                ledFlowRef_.setIsAliveSignaled(true);
                IAliveForwardBehaviour::setMessageReceived(true);
                watcher_->stopAndReset(ProgramConfig::aliveNoResponsReportTOutID);
            }
            break;
        case AliveData::Alive_Confirm:
            // Last member acknowledges the message to the coordinator "alive round trip finished"
            if (incomingAlive.getIncomingGroup() == IRoutingManager::BGEndAck) {
                sendLastAckReply();
            }
            watcher_->stopAndReset(repeatTimeOut_.id);
            watcher_->stopAndReset(ProgramConfig::rAliveJumpTOutID);
            break;
        case AliveData::Alive_Indication:
            // forwardIfNoReaction(incomingAlive);  // Forwarding is controlled by the coordinator
            break;
    case AliveData::Alive_RoundTrip:
        outgoingAlive_->setRoundTripID(incomingAlive.getRoundTripID());
        sendLastAckMessage(true);
        break;
    case AliveData::Alive_Unknown:
        break;
    }
}



void BGroupForwarding::forwardAliveMessage(AliveData& incomingAlive) {
    setAliveOutGroupBySearchRequest(incomingAlive, IRoutingManager::SEARCH_TRANSMITTER_IP_BY_RECEIVER_IP);
    saveReverseRouteInfo(incomingAlive);
    uint16_t targetIP = aliveMessageContainer.targetIP;
    aliveMessageContainer.retransmitGroup = targetIP;
    if (targetIP != IRoutingManager::BGEnd && targetIP != IRoutingManager::BGEndAck) {
        defaultForwardGroup = targetIP;
    }
    outgoingAlive_ = &incomingAlive;
  outgoingAlive_->incrementPassedNodes();
    lastRoundTripId_ = outgoingAlive_->getRoundTripID();
    sendAliveAndReset();
}

void BGroupForwarding::forwardIfNoReaction(__attribute__((unused)) AliveData& incomingAlive) {
    /*** Forwarding is controlled by the coordinator
     enum {indicationOnTopOffset = 5};
     uint8_t repeatTimeOutAmount =  watcher_->findTimeOutWithID(ProgramConfig::rAliveTOutID)->timeOutAmount;
     if (++indicationCount_> (repeatTimeOutAmount + indicationOnTopOffset)) {
         setAliveOutGroupBySearchRequest(incomingAlive,IRoutingManager::SEARCH_TRANSMITTER_IP_BY_LISTENER_IP);
         if(aliveMessageContainer.targetIP == IRoutingManager::BGAdmin) {
             sendLastAckReply();
         } else {
             sendAliveAndReset();
         }
     }
     ***/
}

// Coordinator: inform the last peer, that the actual alive round trip has ended.
void BGroupForwarding::sendLastAckMessage(bool firstTry) {
    watcher_->stopAndReset(repeatTimeOut_.id);
    if (firstTry) {
        watcher_->stopAndReset(repeatAliveEndTimeout_.id);
        watcher_->start(repeatAliveEndTimeout_.id);
    }
    aliveMessageContainer.targetIP = IRoutingManager::BGEndAck;  // ToDo(AME): determine correct IP
                                                                  // (look at SectionGuard, final alive end message)
    outgoingAlive_->setOutgoingGroup(IRoutingManager::BGEndAck);
    presetAliveText(outgoingAlive_);
    transmitAliveText();
    // setEnable(false);
}

// Last peer in the alive chain send acknowledge after receiving of BGEndAck
void BGroupForwarding::sendLastAckReply() {
    int len = snprintf(aliveMessageContainer.text, sizeof(aliveMessageContainer.text),
            "%s%c:%03d", ModemInterpreter::commandPrefix,
            ModemInterpreter::AliveLastAckReply, lastRoundTripId_);  // ::B:012
    // Send message. Compare with transmitAliveText();
    watcher_->stopAndReset(ProgramConfig::rAliveJumpTOutID);
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            (const uint8_t*) aliveMessageContainer.text, (uint8_t) len, IRoutingManager::BCAliveIP, false,
            TermContextFilter::filterAliveTxInfo);
}

// Coordinator: Acknowledge of last peer corresponding to the BGEndAck message received.
// Input tripID: reported Alive RoundTrip-ID with ::B-Message
//   >=0 compare tripID with lastRoundTripId_
//   <0  ignore tripID
void BGroupForwarding::handleLastAckReply(int tripID) {
  bool msg = false;
  if (ProgramConfig::isCoordinator()) {
    watcher_->stopAndReset(repeatAliveEndTimeout_.id);
    watcher_->stopAndReset(ProgramConfig::aliveJumpRequestTOutID);
    ITimeOutWatcher::timeOutInfo* pToi = watcher_->findTimeOutWithID(ProgramConfig::aliveACKReplyOnceID);
    if (pToi) {
      if (pToi->started) {
        pToi->timeCounter = pToi->delay;  // re-trigger timer
        msg = true;
      } else {
          watcher_->stopAndReset(ProgramConfig::aliveACKReplyOnceID);
          watcher_->start(ProgramConfig::aliveACKReplyOnceID);
          internalSetEnabled(false);
      }
    } else {
        internalSetEnabled(false);
    }
  } else {  // @Peer
      if (tripID >= 0) {
          if (tripsGreaterThanLast((uint8_t) tripID)) {
              msg = true;
          } else {
              watcher_->stopAndReset(repeatTimeOut_.id);
          }
      } else {
          watcher_->stopAndReset(repeatTimeOut_.id);
      }
  }
  if (msg && termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveTxInfo)) {
    termFilterPtr_->println(TX::RepeatedTreatmentAckAlive);
  }
  if (!enabled_ && termFilterPtr_->isNotFiltered(TermContextFilter::filterIntegrationTest)) {
      termFilterPtr_->println(TX::dTxtIntegration008);
  }
}


// @Peer: Suppress repeating messages for the alive to the following peers if the following condition is met:
// - alive messages which are for a peer with a higher routing number than the own routing number
// - having the same round trip number in compare to the registered before
// 15.04.2024 DS The routine did not have the desired effect or was even disruptive to the alive round trips
// void BGroupForwarding::handleSuspendRetransmit(AliveData& incomingAlive) {
//  uint8_t memberID = incomingAlive.getMemberID();
//  if (memberID == IRoutingManager::Transmitter &&
//      incomingAlive.getIncomingGroup()>defaultForwardGroup &&
//      incomingAlive.getRoundTripID() == lastRoundTripId_ ) {
//      watcher_->stopAndReset(repeatTimeOut_.id);
//      watcher_->stopAndReset(ProgramConfig::rAliveJumpTOutID);
//      if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveTxInfo)) {
//          termFilterPtr_->println("handleSuspendRetransmit.");
//      }
//  }
// }


void BGroupForwarding::setAliveOutGroupBySearchRequest(AliveData &incomingAlive,
                        IRoutingManager::dataKey searchRequest) {
    routingManager_.storeData(IRoutingManager::SEARCH_Value, incomingAlive.getIncomingGroup());
    uint16_t outGroup = routingManager_.getData(searchRequest);
    incomingAlive.setOutgoingGroup(outGroup);
    aliveMessageContainer.targetIP = outGroup;
}


void BGroupForwarding::saveReverseRouteInfo(AliveData& incomingAlive) {
    routingManager_.storeData(IRoutingManager::SEARCH_Value, incomingAlive.getIncomingGroup());
    bypassInfo.bypassReceiverIp =
            routingManager_.getData(IRoutingManager::SEARCH_IP_FOR_SAME_ROLE_IN_FOLLOWING_SECTION);
}

void BGroupForwarding::sendAliveAndReset() {
    sendAliveData();
    resetTimeOuts();
    indicationCount_ = 0;
}

void BGroupForwarding::sendAliveData(bool isGlobalTransmission) {
    if (((aliveMessageContainer.targetIP != 0xFF && aliveMessageContainer.targetIP != 0) || isGlobalTransmission) &&
            outgoingAlive_ !=NULL) {
        outgoingAlive_->setMemberID(IRoutingManager::Transmitter);
        outgoingAlive_->setOutgoingGroup(aliveMessageContainer.targetIP);
        presetAliveText(outgoingAlive_);
        transmitAliveText();
    }
}

void BGroupForwarding::presetAliveText(char *output, uint8_t memberID, uint8_t outgoingGroup,
                            AliveData::dataField_t df) {
    snprintf(output, AliveMsgSize, "%s%03u:%03u:%08x", AliveData::alivePrefix, memberID,
            outgoingGroup, (unsigned int) df.all);
}

void BGroupForwarding::presetAliveText(AliveData *refAlive) {
    snprintf(aliveMessageContainer.text, AliveMsgSize, "%s%03u:%03u:%08x", refAlive->alivePrefix,
            refAlive->getMemberID(), refAlive->getOutgoingGroup(), (unsigned int) refAlive->getDataField());
}

void BGroupForwarding::transmitAliveText() {
    int32_t checkGroup = atoi((const char*) (&aliveMessageContainer.text[7]));
    if (checkGroup == 0) {  // ToDo: check why checkGroup is sometimes 0
        if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveTxInfo)) {
            termFilterPtr_->println(TX::FctnTransmitAliveTextErr);
            termFilterPtr_->println(aliveMessageContainer.text);
        }
        return;
    }
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            (const uint8_t*) aliveMessageContainer.text, 3 + 3 + 1 + 3 + 1 + 8,
            IRoutingManager::BCAliveIP, false,
            TermContextFilter::filterAliveTxInfo);
}


void BGroupForwarding::resetTimeOuts() {
    watcher_->stopAndReset(repeatTimeOut_.id);
    watcher_->start(repeatTimeOut_.id);
}

void BGroupForwarding::setEnable(bool status) {
    internalSetEnabled(status);
}

void BGroupForwarding::internalSetEnabled(bool status) {
    enabled_ = status;
    if (!enabled_) {    // current roundtrip is finished
        incOrClearTimeout();
    }
}


bool BGroupForwarding::getEnableStatus() {
    return enabled_;
}

void BGroupForwarding::start() {
    *outgoingAlive_ = AliveData(AliveData::outgoing, IRoutingManager::BGRoundStart, IRoutingManager::Transmitter);
    outgoingAlive_->setRoundTripID((uint8_t) (++lastRoundTripId_));
    outgoingAlive_->setPassedNodes(0);
    aliveMessageContainer.targetIP = IRoutingManager::BGRoundStart;
    aliveMessageContainer.retransmitGroup = IRoutingManager::BGRoundStart;
    clearErrorFlags();
    JumpMapper::prepareNextRoundTrip();
    sendAliveAndReset();
}



void BGroupForwarding::retransmitRequest() {
    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveTxInfo)) {
        char buf[60];
        snprintf(buf, sizeof(buf), TX::getText(TX::AliveRetransmitRequest), ProgramConfig::rAliveTOutID);
        termFilterPtr_->println(buf);
    }

    if (outgoingAlive_) {
        outgoingAlive_->incRetries();    // increment number of retries
        outgoingAlive_->setOutgoingGroup(aliveMessageContainer.retransmitGroup);
        presetAliveText(outgoingAlive_);
    }
    transmitAliveText();
}

void BGroupForwarding::handleAliveNoResponseTimeOut() {
    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveTxInfo)) {
        char buf[60];
        snprintf(buf, sizeof(buf), TX::getText(TX::AliveNoResponseTimeout), ProgramConfig::rAliveJumpTOutID);
        termFilterPtr_->println(buf);
    }

    BGroupForwarding::alarmStatusCode alarmState = classifyAlarmStatus();
    reactOnAlarmState(alarmState);
}


void BGroupForwarding::retransmitLastAckRequest() {
    sendLastAckMessage(false);
}

void BGroupForwarding::retransmitLastAckCallout() {
    watcher_->stopAndReset(repeatAliveEndTimeout_.id);
    enabled_ = false;   //  setEnable(false);
}


enum BGroupForwarding::alarmStatusCode BGroupForwarding::classifyAlarmStatus() {
    BGroupForwarding::alarmStatusCode statusCode;
    if (ProgramConfig::isCoordinator()) {
        statusCode = CoordLostCon;
    } else {
        statusCode = PeerLostCon;
    }
    return statusCode;
}

void BGroupForwarding::reactOnAlarmState(BGroupForwarding::alarmStatusCode alarmState) {
    switch (alarmState) {
    case CoordLostCon:
        JumpMapper::setMappingByReporter(0, outgoingAlive_->getRoundTripID());
        JumpMapper::informUserAboutLostIPByReporter();
        watcher_->stopAndReset(ProgramConfig::rAliveTOutID);
        break;
    case PeerLostCon:
        watcher_->start(ProgramConfig::aliveNoResponsReportTOutID);
        break;
    case ReversCoordLostCon:  // <---- is actually obsolete 06.03.2024
    case ReversPeerLostCon:
        isReversRouteAlerted_ = true;
        break;
    default:
    case undefined:
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::GroupAdrErrClassSpecification), outgoingAlive_->getOutgoingGroup());
        ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(
                Global2::OutBuff, 0, false);
    }
}


void BGroupForwarding::transmitAlarmRequest() {
    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveTxInfo)) {
        char buf[60];
        snprintf(buf, sizeof(buf), TX::getText(TX::AliveTransmitAlarmRequest),
                 ProgramConfig::aliveNoResponsReportTOutID);
        termFilterPtr_->println(buf);
    }

    auto &prefix = ModemInterpreter::commandPrefix;
    uint16_t myIp = ModemController::getNetworkIPAddress();
    if (myIp != 0) {
        uint8_t tripID = outgoingAlive_->getRoundTripID();
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%c:%03u:%03u", prefix,
                ModemInterpreter::errorAliveNoResponseRequest, myIp, tripID);
        ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                reinterpret_cast<uint8_t*>(Global2::OutBuff), (uint8_t) strlen(Global2::OutBuff),
                IRoutingManager::BCGlobalIP, true, TermContextFilter::filterRoutingInfo);
    }
}


void BGroupForwarding::handleTransmissionFailure() {
    if (isReversRouteAlerted_ & isReverseRoutingEnabled_) {
        triggerReversRoute();
        isReversRouteAlerted_ = false;
    }
}


void BGroupForwarding::triggerReversRoute() {
    AliveData incomingAlive = AliveData(AliveData::incoming, bypassInfo.bypassReceiverIp, IRoutingManager::Receiver);
    incomingAlive.setStatus(AliveData::Alive_Request);
    reactOnAliveWithHostID(incomingAlive);
}


// Checks, if the round trip amount is greater than the stored round trip number lastRoundTripId_.
// The routine takes overflows into account
bool BGroupForwarding::tripsGreaterThanLast(uint32_t compare) {
    int32_t limit = 1 << (AliveData::Trips - 2);        // 64
    int32_t full = 1 << (AliveData::Trips);             // 256
    int32_t last = (int32_t) ((uint32_t) lastRoundTripId_);
    int32_t c2 = (int32_t) compare;
    if ((c2 < limit) && (last > limit)) {
        last -= full;
    }
    if ((c2 > limit) && (last < limit)) {
        c2 -= full;
    }
    int32_t delta = c2 - last;
    return (delta > 0);
}


// Clear all error flags for the members who participate in the alive round trips
void BGroupForwarding::clearErrorFlags() {
    for (int i=1; i <= ModemController::networkMemberMax; i++) {
        NetworkMember *pMember = ModemController::getNetMemberPtrByIndex((uint16_t) i);
        if (pMember && pMember->getLink() != NetworkMember::disconnected) {
            if (pMember->getAliveMember()) {
                pMember->setAliveError(false);
            }
        }
    }
}

// Increments or clears timeout variable depending on the error flag
void BGroupForwarding::incOrClearTimeout() {
    for (int i=1; i <= ModemController::networkMemberMax; i++) {
        NetworkMember *pMember = ModemController::getNetMemberPtrByIndex((uint16_t) i);
        if (pMember && pMember->getLink() != NetworkMember::disconnected) {
            if (pMember->getAliveMember()) {
                if (pMember->getAliveError()) {
                   // suppress overrun
                   uint16_t to = pMember->getAliveTimeout();
                   if (to != 0xFFFE) {
                       pMember->setAliveTimeout((uint16_t) (to+1));
                   }
                } else {
                    pMember->setAliveTimeout(0);
                }
            }
        }
    }
}


BGroupForwarding::~BGroupForwarding() {
}


}  // namespace AME_SRC
