/*
 * ForwardingFlow.h
 *
 *  Created on: Aug 4, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_FORWARDINGFLOW_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_FORWARDINGFLOW_H_

#include <cstdint>

#include "IAliveForwardBehaviour.h"
#include "AliveData.h"
#include "../../../CompositeComponents/FoundationModule/SimpleTimeOutWatcher.h"
#include "../../IoCommandManager.h"
#include "../../../Terminal/TermContextFilter.h"

namespace AME_SRC {

class ForwardingFlow {
 public:
    ForwardingFlow(IAliveForwardBehaviour * behaviourPtr, IoCommandManager& ioCmdMangerReff, ITimeOutWatcher& watcher,
            SerialDriver *notifyChannel);
    void processForwarding();
    void updateRoundTrip();
    void window60s(bool isCoordinator);
    void handleTransmissionFailure();
    virtual ~ForwardingFlow();
    void loadAndProofAliveCmd(const char* commandStr, uint16_t len);

    static char* terminalStartAlive(const char *text);
    static char* callStartAliveRoundTrips(const char *text, bool force, bool *success);
        static void startAliveRoundTrip();
        static void initRoundTripTimeouts();
    static char* startStopAliveRoundTrips(char *text);
    static char* callStopAliveRoundTrips();

    bool isAliveCmdValid() {
        return aliveValidStatus_;
    }

    uint16_t getReceivedIp() const {
        return receivedIP_;
    }

    void setReceivedIp(uint16_t recivedIp) {
      receivedIP_ = recivedIp;
    }

    static bool isRoundTripFinished() {
        return roundTripFinished || (roundTripStopDelay > 0);
    }

    static void setRoundTripFinished(bool value) {
      roundTripFinished = value;
    }

    uint32_t getRoundTripCount() {
        return roundTripCount;
    }


    uint32_t getRoundTripRepeatCallOuts() {
        return roundTripRepeatCallouts;
    }

    static void setRoundTripStopDelay(int8_t set) {
      roundTripStopDelay = set;
    }


    static uint8_t getAliveStartId() {
        return aliveStartID_;
    }

    static void setAliveStartId(uint8_t aliveStartId) {
        aliveStartID_ = aliveStartId;
    }

 private:
    static uint8_t aliveStartID_;
    static IAliveForwardBehaviour *behaviourPtr_;
    IoCommandManager &ioCmdMangerReff_;
    static SimpleTimeOutWatcher::timeOutInfo roundTripMonitorTimeOut_;
    static ITimeOutWatcher *watcher_;
    static TermContextFilter *termFilterPtr_;

    bool aliveValidStatus_;
    const char *aliveStrPointer_;
    uint16_t aliveStrLen_;
    uint16_t receivedIP_;
    static uint32_t roundTripCount;
    static uint16_t roundTripAmount_;
    static uint16_t roundTripRepeatCallouts;
    static bool roundTripFinished;
    static char aliveJumpRequest[16];
    static uint16_t aliveJumpDestIP;
    static uint16_t pollTargetGroup_;
    static int8_t roundTripStopDelay;  // Dwell time to stop the alive round trips in minutes
    AliveData* createAliveDataFromCmd(const char *commandStr, uint16_t len);

    void monitorRemainingRoundTrips();

    void informAboutRoundTripProgress();

    static void informAboutRoundTripTimeOut();
    static void roundTripRepeatCallout();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_FORWARDINGFLOW_H_
