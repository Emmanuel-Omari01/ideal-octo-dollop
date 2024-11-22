/*
 * jumpMapper.cpp
 *
 *  Created on: Mar 6, 2024
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "JumpMapper.h"

#include "../../../SystemBase/ProgramConfig.h"
#include "BGroupForwarding.h"

namespace AME_SRC {

JumpMapper::jumpMapping JumpMapper::nextJumpMapping_ = {0, 0, 0, 0, 0, 0, 0, false};
char JumpMapper::aliveJumpMessageContainer_[BGroupForwarding::AliveMsgSize] = {0};
AssignmentFlow* JumpMapper::assignFlowPtr_ = NULL;
TermContextFilter *JumpMapper::termFilterPtr_;
ITimeOutWatcher* JumpMapper::watcher_ = NULL;

uint16_t JumpMapper::lostIp_ = 0;

JumpMapper::JumpMapper(ITimeOutWatcher* watcherPtr, SerialDriver *notifyChannel, AssignmentFlow &assignFlowRef):
        aliveJumpRequestTOut_{new ITimeOutWatcher::timeOutInfo()},
        repeatJumpTimeOut_{new ITimeOutWatcher::timeOutInfo()},
        timeOutState_{idle}
{
    isNotLastInRoute_ = false;
    isNoOtherJumpRequest_ = false;
    isNoDoubledRequest_ = false;
    assignFlowPtr_ = &assignFlowRef;
    termFilterPtr_ = new TermContextFilter(notifyChannel);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    watcher_ = watcherPtr;
    ITimeOutWatcher::createDefaultTimeOut(aliveJumpRequestTOut_,
                                          ProgramConfig::aliveJumpRequestTOutID,
                                          &transmittAliveJumpMessageContainer, &aliveJumpRequestCallout);
    aliveJumpRequestTOut_->timeCounter = 40;
    aliveJumpRequestTOut_->delay = 40;
    aliveJumpRequestTOut_->timeOutCounter = 3;
    aliveJumpRequestTOut_->timeOutAmount = 3;
    aliveJumpRequestTOut_->callOutAmount = 1;
    aliveJumpRequestTOut_->callOutCounter = 1;
    watcher_->registerTimeout(*aliveJumpRequestTOut_);
    watcher_->stopAndReset(ProgramConfig::aliveJumpRequestTOutID);

    ITimeOutWatcher::createTripleTimeOut(repeatJumpTimeOut_, ProgramConfig::rAliveJumpTOutID,
                                         &transmittAliveJumpMessageContainer, &BGroupForwarding::handleAliveNoResponseTimeOut);
    watcher_->registerTimeout(*repeatJumpTimeOut_);
    watcher_->stopAndReset(ProgramConfig::rAliveJumpTOutID);
}


JumpMapper::~JumpMapper() = default;

// @Peer: reaction to the request (receive of "::Z") to proceed the alive round trip
void JumpMapper::reactOnJumpRequest(uint16_t destGroupID, uint16_t roundtripID) {
    if (isTimeOutIDValidAndRunning(ProgramConfig::rAliveJumpTOutID)) {
        forwardAliveJump2GroupWithID((uint16_t) destGroupID, (uint8_t) roundtripID);
        watcher_->start(ProgramConfig::rAliveJumpTOutID);
    } else {
        informAboutTimeOutInvalidity(ProgramConfig::rAliveJumpTOutID);
    }
}

// @Coordinator: reaction to the request (receive of "::E") of a peer (errorReporterIp), who
// informs that his next peer in not reachable
void JumpMapper::reactOnAliveNoResponseRequest(uint16_t errorReporterIp, uint8_t roundTripID) {
    setMappingByReporter(errorReporterIp, roundTripID);
}

// @Coordinator: Call out-Routine in case of repeated failure of the peer "z" message
// => address next peer in the alive chain
void JumpMapper::aliveJumpRequestCallout() {
    if (ProgramConfig::isCoordinator() && !ForwardingFlow::isRoundTripFinished()) {
        uint16_t index = nextJumpMapping_.calloutIndex;
        uint16_t refIdx = AssignmentFlow::getAdjazenzParent()->getRouteListEntries(false);
        uint16_t nextIP = 0;

        informUserAboutLostIPByReporter();

        if (index < refIdx) {
            nextIP = assignFlowPtr_->getRoutingIPForIndex(index);
            if (nextIP) {
                lostIp_ = nextIP;
                nextJumpMapping_.targetIP = nextIP;
                nextJumpMapping_.calloutHandling = true;
                nextJumpMapping_.reporterIP = 0;
                if (assignFlowPtr_->isLastRoutingIP((uint16_t) nextJumpMapping_.targetIP)) {
                    nextJumpMapping_.targetRoutingIndex = IRoutingManager::BGEnd;
                } else {
                    nextJumpMapping_.targetRoutingIndex = (uint16_t) (AssignmentFlow::getRoutingIndexForIp(
                                    nextJumpMapping_.targetIP) + 1);
                }
                ModemInterpreter::setInterpretState(
                        ModemInterpreter::aliveReEntryMessage);  // => transmitJumpRequestAndInform() after 100ms
            }
        }
        if (nextIP) {
            nextJumpMapping_.calloutIndex++;
            ITimeOutWatcher::timeOutInfo *toi = watcher_->findTimeOutWithID(ProgramConfig::aliveJumpRequestTOutID);
            if (toi != NULL) {
                toi->started = true;
            }
        } else {
            BGroupForwarding::handleLastAckReply(-1);
        }
    }
}

void JumpMapper::setMappingByReporter(uint16_t errorReporterIp, uint8_t roundTripID) {
    nextJumpMapping_.reporterIP = errorReporterIp;
    nextJumpMapping_.triggerIP = errorReporterIp;
    nextJumpMapping_.roundTripID = roundTripID;
    lostIp_ = assignFlowPtr_->getNextRouteIpAfterIp((uint16_t) errorReporterIp);
    ModemController::setMemberErrorByIdx(lostIp_, true);
    uint16_t nextIP = AssignmentFlow::getNextRouteIpAfterIp(lostIp_);
    nextJumpMapping_.targetIP = (errorReporterIp == 0) ? nextIP : lostIp_;
    if (nextIP != 0xFF) {
        nextJumpMapping_.calloutIndex = AssignmentFlow::getRoutingIndexForIp(nextIP);
        nextJumpMapping_.calloutHandling = false;
    }
    nextJumpMapping_.targetRoutingIndex =
            (uint16_t) (AssignmentFlow::getRoutingIndexForIp(nextJumpMapping_.targetIP) + 1);
    if (assignFlowPtr_->isPreLastRoutingIP((uint16_t) errorReporterIp)) {
        BGroupForwarding::handleLastAckReply(-1);
    } else {
        if (nextJumpMapping_.targetIP != 0) {
            ModemInterpreter::setInterpretState(
                    ModemInterpreter::aliveReEntryMessage);  // => transmitJumpRequestAndInform() after 100ms
        }
    }
}


// @Coordinator: Actions to prepare for the next round trip
void JumpMapper::prepareNextRoundTrip() {
    nextJumpMapping_.triggerIP = 0xFFFF;  // Initialization of isErrorReporterRequestValid:
                                          // set invalid IP to isNoDoubledRequest_
}


bool JumpMapper::isErrorReporterRequestValid(uint16_t errorReporterIp, uint8_t tripID) {
    isNotLastInRoute_ = !assignFlowPtr_->isLastRoutingIP((uint16_t) errorReporterIp);
    isNoOtherJumpRequest_ = isTimeOutIDValidAndRunning(ProgramConfig::aliveJumpRequestTOutID);
    isNoDoubledRequest_ = !(errorReporterIp == nextJumpMapping_.triggerIP && tripID == nextJumpMapping_.roundTripID);
    if (!isNoOtherJumpRequest_) {
        informAboutTimeOutInvalidity(ProgramConfig::aliveJumpRequestTOutID);
    }
    return isNotLastInRoute_ && isNoOtherJumpRequest_ && isNoDoubledRequest_;
}


void JumpMapper::informUserAboutJumpInvalidity() {
    if (!isNotLastInRoute_) {
        termFilterPtr_->println(TX::getText(TX::ReporterIsLastMember));
    } else if (!isNoOtherJumpRequest_) {
        termFilterPtr_->println(TX::getText(TX::ReportAnotherAliveLoss));
    } else if (!isNoDoubledRequest_) {
        termFilterPtr_->println(TX::getText(TX::ReportRequestSameIPAndNumber));
    } else {
        termFilterPtr_->println(TX::getText(TX::ReportErrorJumpMapper));
    }
}

void JumpMapper::reply2NoResponseRequestReporter(uint16_t errorReporterIp) {
    snprintf(Global2::InBuff, Global2::inBuffMaxLength, "%s%c:%u", ModemInterpreter::commandPrefix,
            ModemInterpreter::errorAliveNoResponseReply, errorReporterIp);
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t*>(Global2::InBuff), (uint8_t) strlen(Global2::InBuff),
            IRoutingManager::BCGlobalIP, true, 0);
}

// @Coordinator: reaction on "Z" request message (receive of "z" message)
void JumpMapper::reactOnJumpReply() {
    watcher_->stopAndReset(ProgramConfig::aliveJumpRequestTOutID);
    termFilterPtr_->println(TX::AliveJumpReqAck);
}

// @Peer: Transmission of "z" as reaction on reception of "Z"
void JumpMapper::transmitJumpReply() {
    // Here the aliveJumpMessageContainer_ is not used because the usage could block the forwarding process
    snprintf(Global2::InBuff, Global2::inBuffMaxLength, "::%c", ModemInterpreter::aliveJumpReply);
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            (const uint8_t*) Global2::InBuff, (uint8_t) strlen(Global2::InBuff),
            IRoutingManager::BCGlobalIP, true, 0);
}



void JumpMapper::transmitJumpRequestAndInform() {
    ITimeOutWatcher::timeOutInfo *toi = watcher_->findTimeOutWithID(ProgramConfig::aliveJumpRequestTOutID);
    bool isJumpRequestIdleOrCallOut = ((toi != NULL && toi->started == false)
            || nextJumpMapping_.calloutHandling);

    if (isJumpRequestIdleOrCallOut) {
        transmittAliveJumpRequest();
        watcher_->start(ProgramConfig::aliveJumpRequestTOutID);
    }

    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveJumpInfo)) {
        if (isJumpRequestIdleOrCallOut) {
            snprintf(Global2::InBuff, Global2::inBuffMaxLength, TX::getText(TX::StartAliveRoundTripWithIP),
                    nextJumpMapping_.targetIP);
        } else {
            snprintf(Global2::InBuff, Global2::inBuffMaxLength, TX::getText(TX::IgnoreReqDueAlreadyHandled));
        }
        termFilterPtr_->println(Global2::InBuff);
    }
}

void JumpMapper::forwardAliveJump2GroupWithID(uint16_t groupID, uint8_t roundtripID) {
    nextJumpMapping_.targetIP = groupID;
    nextJumpMapping_.bcIP = groupID;
    AliveData::dataField_t df;
    df.all = 0;
    df.bit.roundTripID = roundtripID;
    BGroupForwarding::presetAliveText(aliveJumpMessageContainer_,
            IRoutingManager::Transmitter, (uint8_t) groupID, df);
    transmittAliveJumpMessageContainer();
}

void JumpMapper::transmittAliveJumpRequest() {
    AliveData::dataField_t df;
    df.all = 0;
    df.bit.roundTripID = (uint8_t) (ForwardingFlow::getAliveStartId() + 1);
    ForwardingFlow::setAliveStartId(df.bit.roundTripID);
    snprintf(aliveJumpMessageContainer_, BGroupForwarding::AliveMsgSize, "::%c%03u:%03u:%03u",
            ModemInterpreter::aliveJumpRequest,
            (uint8_t) nextJumpMapping_.targetIP,
            (uint8_t) nextJumpMapping_.targetRoutingIndex,
            (uint8_t) df.bit.roundTripID);
    if (nextJumpMapping_.targetIP != 0) {
        nextJumpMapping_.bcIP = IRoutingManager::BCGlobalIP;
        transmittAliveJumpMessageContainer();
    }
}

void JumpMapper::transmittAliveJumpMessageContainer() {
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            (const uint8_t*) aliveJumpMessageContainer_,
            (uint8_t) strlen(aliveJumpMessageContainer_),
            IRoutingManager::BCAliveIP, true, 0);
}


bool JumpMapper::isTimeOutIDValidAndRunning(uint8_t timeOutID) {
    ITimeOutWatcher::timeOutInfo *toi = watcher_->findTimeOutWithID(timeOutID);
    if (toi) {
        if (toi->started) {
            timeOutState_ = running;
            return false;
        } else {
            timeOutState_ = idle;
            return true;
        }
    }
    timeOutState_ = notInitialized;
    return false;
}

void JumpMapper::informUserAboutLostIPByReporter() {
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::MemberReportsLossOfSuccessor),
             nextJumpMapping_.reporterIP, lostIp_);
    termFilterPtr_->println(Global2::OutBuff);
}

void JumpMapper::informAboutTimeOutInvalidity(uint8_t timeOutID) {
    switch (timeOutState_) {
        case running:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::ErrorJumpMessageReceived), timeOutID);
            break;
        case idle:
            return;
        case notInitialized:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::TimeoutIDNotInitialized), timeOutID);
            break;
    }
    termFilterPtr_->println(Global2::OutBuff);
}

}  // namespace AME_SRC
