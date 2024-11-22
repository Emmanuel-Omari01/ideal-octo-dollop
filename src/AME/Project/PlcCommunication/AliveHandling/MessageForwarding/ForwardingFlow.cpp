/*
 * ForwardingFlow.cpp
 *
 *  Created on: Aug 4, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "ForwardingFlow.h"

#include <cstring>

#include "../../ModemController.h"
#include "../../../HelpStructures/CharOperations.h"
#include "../../../SystemBase/ProgramConfig.h"

namespace AME_SRC {

uint16_t ForwardingFlow::roundTripAmount_ = 0;
uint16_t ForwardingFlow::roundTripRepeatCallouts = 0;
uint32_t ForwardingFlow::roundTripCount = 0;
bool ForwardingFlow::roundTripFinished = false;
uint16_t ForwardingFlow::pollTargetGroup_ = 0;
int8_t ForwardingFlow::roundTripStopDelay = 0;
uint8_t ForwardingFlow::aliveStartID_ = 1;

uint16_t ForwardingFlow::aliveJumpDestIP = 0;
IAliveForwardBehaviour* ForwardingFlow::behaviourPtr_ = NULL;
SimpleTimeOutWatcher::timeOutInfo ForwardingFlow::roundTripMonitorTimeOut_;
ITimeOutWatcher* ForwardingFlow::watcher_ = NULL;
TermContextFilter *ForwardingFlow::termFilterPtr_ = NULL;

ForwardingFlow::ForwardingFlow(IAliveForwardBehaviour * behaviourPtr, IoCommandManager& ioCmdMangerReff,
        ITimeOutWatcher& watcher, SerialDriver *notifyChannel): ioCmdMangerReff_{ioCmdMangerReff},
                                                                aliveValidStatus_{false},
                                                                aliveStrPointer_{0},
                                                                aliveStrLen_{0},
                                                                receivedIP_{0}
{
    behaviourPtr_ = behaviourPtr;
    watcher_ = &watcher;
    termFilterPtr_ = new TermContextFilter(notifyChannel);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);

    ITimeOutWatcher::createDefaultTimeOut(&roundTripMonitorTimeOut_,
                                          ProgramConfig::roundtripMonitorTOutID,
                                          &informAboutRoundTripTimeOut, &roundTripRepeatCallout);

    watcher_->registerTimeout(roundTripMonitorTimeOut_);
    watcher_->stopAndReset(ProgramConfig::roundtripMonitorTOutID);
}

void ForwardingFlow::processForwarding() {
    if (isRoundTripFinished()) {
        return;
    }
    // TODO(AME): extract IP from incoming alive message => invokerIP
    AliveData *aliveCandidate = createAliveDataFromCmd(aliveStrPointer_, aliveStrLen_);
    if (aliveCandidate != NULL) {
        behaviourPtr_->forward(*aliveCandidate);
        delete aliveCandidate;
    }
}

void ForwardingFlow::updateRoundTrip() {
    if (behaviourPtr_->getEnableStatus() == false) {
        monitorRemainingRoundTrips();
    }
}

void ForwardingFlow::monitorRemainingRoundTrips() {
    if (roundTripAmount_ != 0 && !roundTripFinished) {
        if (roundTripAmount_ != 0xFFFF && roundTripCount >= roundTripAmount_) {
          roundTripFinished = true;
            watcher_->stopAndReset(ProgramConfig::roundtripMonitorTOutID);
        } else {
            roundTripCount++;
            behaviourPtr_->start();
            behaviourPtr_->setEnable(true);
            watcher_->stopAndReset(ProgramConfig::roundtripMonitorTOutID);
            watcher_->start(ProgramConfig::roundtripMonitorTOutID);
        }
      informAboutRoundTripProgress();
    }
}

void ForwardingFlow::window60s(bool isCoordinator) {
    if (roundTripFinished && roundTripStopDelay >= 0) {
        if (--roundTripStopDelay <= 0 && !isCoordinator) {
          roundTripFinished = false;
        }
    }
}


void ForwardingFlow::loadAndProofAliveCmd(const char *commandStr, uint16_t cmdLen) {
    enum { patternLen = 6 };
    for (uint16_t cmdIdx = 0; cmdIdx < (cmdLen - (patternLen - 1)); cmdIdx++) {
        if (commandStr[cmdIdx] == ':') {
            char *positionPtr = strstr(commandStr + cmdIdx, AliveData::alivePrefix + 1);
            if (positionPtr != 0) {
                aliveStrPointer_ = positionPtr;
                aliveStrLen_ = (uint16_t) (cmdLen - cmdIdx);
                aliveValidStatus_ = true;
                return;
            }
        }
    }
    aliveValidStatus_ = false;
}

char* ForwardingFlow::terminalStartAlive(const char *text) {
    bool temp;
    return callStartAliveRoundTrips(text, false, &temp);
}

char* ForwardingFlow::callStartAliveRoundTrips(const char *text, bool force, bool *success) {
    unsigned int cnt;
    int roundTrips;

    *success = false;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%d", &roundTrips);

    if (ProgramConfig::isCoordinator()) {
        if (ProgramConfig::getIntegrationTestPtr()->isAliveTestRunning() && !force) {
            return const_cast<char *>(TX::getText(TX::AliveWarningSuperiorTest));
        }
        if (cnt == 1) {
            roundTripAmount_ = (uint16_t) (roundTrips & 0xFFFF);
            roundTripCount = 0;
            if (roundTripAmount_) {
                roundTripRepeatCallouts = 0;
              startAliveRoundTrip();
                *success = true;
                return const_cast<char *>(TX::getText(TX::RoundTripsAreStarted));
            } else {
                return callStopAliveRoundTrips();
            }
        } else {
            return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
        }
    } else {
        return const_cast<char *>(TX::getText(TX::UseCommandInCoordinatorModeOnly));
    }
}

char *ForwardingFlow::callStopAliveRoundTrips() {
    if (ProgramConfig::isCoordinator()) {
        IntegrationsTests *pTest = ProgramConfig::getIntegrationTestPtr();
        if (pTest->isAliveTestRunning()) {
            pTest->testAliveStop();
            return const_cast<char *>("");
        }
      roundTripFinished = true;
        ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t *>(const_cast<char*>("::a000")), 6,
            IRoutingManager::BCGlobalIP, false,
            TermContextFilter::filterAliveTxInfo);
        watcher_->stopAndReset(ProgramConfig::roundtripMonitorTOutID);
        return const_cast<char *>(TX::getText(TX::RoundTripsAreStopped));
    } else {
        return const_cast<char *>(TX::getText(TX::UseCommandInCoordinatorModeOnly));
    }
}

// Start and restart alive round trips
void ForwardingFlow::startAliveRoundTrip() {
  roundTripFinished = false;
    behaviourPtr_->start();
  initRoundTripTimeouts();
    watcher_->start(ProgramConfig::roundtripMonitorTOutID);
}

void ForwardingFlow::initRoundTripTimeouts() {
    uint16_t peersConnected = ModemController::getNumPeersConnected(false);
    SimpleTimeOutWatcher::timeOutInfo *toi = watcher_->findTimeOutWithID(
        ProgramConfig::roundtripMonitorTOutID);
    toi->delay = 200;  // 50ms * 200 = 10s
    toi->timeOutAmount = (uint8_t) peersConnected;
    toi->timeCounter = (uint8_t) peersConnected;
    toi->callOutAmount = 1;
    toi->callOutCounter = 1;
    watcher_->stopAndReset(ProgramConfig::roundtripMonitorTOutID);

    toi = watcher_->findTimeOutWithID(ProgramConfig::aliveACKReplyOnceID);
    if (toi) {
        uint32_t delay = (peersConnected + 1) * 2;  // 100ms per Peer
        if (delay > 40) {   // limited to 40 * 50ms = 2000ms
            delay = 40;
        }
        toi->delay = (uint16_t) delay;
        if (!toi->started) {
            toi->timeCounter = (uint16_t) delay;
        }
    }
}

char* ForwardingFlow::startStopAliveRoundTrips(char *text) {
    unsigned int cnt, enable;
    const char *txt = charOperations::skip_token(text, ' ');
    cnt = sscanf(txt, "%u", &enable);

    if (ProgramConfig::isCoordinator()) {
        return const_cast<char*>(TX::getText(TX::UseCommandInPeerModeOnly));
    } else {
        if (cnt == 1 && enable <= 1) {
          roundTripFinished = static_cast<bool>(enable != 0) ? false : true;
          roundTripStopDelay = 0;
        }
        if (roundTripFinished) {
            return const_cast<char*>(TX::getText(TX::RoundTripsAreStopped));
        } else {
            return const_cast<char*>(TX::getText(TX::StartingOfTripsPossible));
        }
    }
}

void ForwardingFlow::handleTransmissionFailure() {
    behaviourPtr_->handleTransmissionFailure();
}

#pragma GCC diagnostic ignored "-Wstack-usage="
AliveData* ForwardingFlow::createAliveDataFromCmd(const char *commandStr, __attribute__((unused))  uint16_t len) {
    int cnt;                                   //                       0123456789ABCDEF01
    unsigned int memberID, incomingGroup, df;  // Example commandStr: ::A001:002:00000001
    // AliveData::dataField_t df;              // commandStr Pointer---^
    cnt = sscanf(commandStr + 2, "%03u:%03u:%08x", &memberID, &incomingGroup, &df);
    if (cnt == 3) {
        AliveData *alive = new AliveData(AliveData::incoming, (uint16_t) incomingGroup, (uint8_t) memberID);
        alive->setDataField(df);
        return alive;
    } else {
        return NULL;
    }
}

ForwardingFlow::~ForwardingFlow() = default;

void ForwardingFlow::informAboutRoundTripProgress() {
    bool finished = isRoundTripFinished();
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowActualRoundTripNumber),
            (unsigned int) getRoundTripCount(),
            (finished) ? TX::getText(TX::StateFinished) : TX::getText(TX::StateInProgress));
    termFilterPtr_->println(Global2::OutBuff);
    if (finished) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                "========================= %s =========================", TX::getText(TX::ShowAllRoundTripsFinished));
        termFilterPtr_->println(Global2::OutBuff);
    }
}

void ForwardingFlow::informAboutRoundTripTimeOut() {
    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveJumpInfo)) {
        SimpleTimeOutWatcher::timeOutInfo *toi = watcher_->findTimeOutWithID(ProgramConfig::roundtripMonitorTOutID);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::TimeoutRoundTripControl), toi->timeOutCounter, toi->timeOutAmount);
        termFilterPtr_->println(Global2::OutBuff);
    }
}

void ForwardingFlow::roundTripRepeatCallout() {
    roundTripRepeatCallouts++;
  startAliveRoundTrip();
}

}  // namespace AME_SRC
