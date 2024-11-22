/*
 * BGroupForwarding.h
 *
 *  Created on: Aug 4, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_BGROUPFORWARDING_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_BGROUPFORWARDING_H_

#include "../../ModemController.h"
#include "IAliveForwardBehaviour.h"
#include "IRoutingManager.h"
#include "JumpMapper.h"
#include "../../../CompositeComponents/FoundationModule/SimpleTimeOutWatcher.h"
#include "../../../SignalProcessing/Aktorik/LED/LEDSignalFlow.h"
#include "../../../Terminal/TermContextFilter.h"

namespace AME_SRC {


class BGroupForwarding: public IAliveForwardBehaviour {
 public:
    BGroupForwarding(SerialDriver *notifyRef, IRoutingManager &routingManager,
            ITimeOutWatcher &watcher, LEDSignalFlow &ledFlowRef);
    void forward(AliveData &incomingAlive);
    virtual ~BGroupForwarding();
    void setEnable(bool status);
    bool getEnableStatus();
    void start();
    void handleTransmissionFailure();
    static void handleLastAckReply(int tripID);
    static void handleAliveNoResponseTimeOut();
    static void presetAliveText(AliveData *refAlive);
    static void presetAliveText(char *output, uint8_t memberID,
            uint8_t outgoingGroup, AliveData::dataField_t df);
    static void transmitAliveText();

    void setLastRoundTripId(uint8_t lastRoundTripId) {
        lastRoundTripId_ = lastRoundTripId;
    }

    enum {
        AliveMsgSize = 22
    };

 private:
    static bool enabled_;
    static bool isReverseRoutingEnabled_;  // Describes during routing if the reverse route has to be considered.
    static bool isReversRouteAlerted_;
    static SimpleTimeOutWatcher::timeOutInfo repeatTimeOut_;
    static SimpleTimeOutWatcher::timeOutInfo noResponseReportTimeOut_;
    static SimpleTimeOutWatcher::timeOutInfo repeatAliveEndTimeout_;
    static TermContextFilter *termFilterPtr_;
    IRoutingManager &routingManager_;
    LEDSignalFlow &ledFlowRef_;
    static ITimeOutWatcher *watcher_;
    uint8_t indicationCount_;
    static uint8_t lastRoundTripId_;
    uint16_t defaultForwardGroup;   // own route number without BGEnd, BGEndAck process
    static AliveData *outgoingAlive_;
    static struct aliveContainer {  // 012345678901234567890
        char text[AliveMsgSize];    // ::Aiii:ttt:dddddddd
                                    // ::A - Prefix, iii = MemberID, ttt=targetIP, dddddddd = dataField
        uint16_t targetIP;
        uint16_t retransmitGroup;
    } aliveMessageContainer;

    typedef struct {
        IRoutingManager::eTupelRoles role;
        uint16_t bypassReceiverIp;
    } endTransmitterBypass;
    static endTransmitterBypass bypassInfo;

    uint8_t getHostRoleIDForIncomingGroup(uint16_t incomingGroup);
    AliveData::aliveType classifyAliveWithIDs(uint8_t hostID, uint8_t memberID);
    void reactOnAliveWithHostID(AliveData&);
    void forwardAliveMessage(AliveData &incomingAlive);
    void forwardIfNoReaction(AliveData &incomingAlive);
    static void sendLastAckMessage(bool firstTry);
    void sendLastAckReply();
    void setAliveOutGroupBySearchRequest(AliveData &incomingAlive,
            IRoutingManager::dataKey searchRequest);
    void saveReverseRouteInfo(AliveData &incomingAlive);
    void sendAliveAndReset();
    void sendAliveData(bool isGlobalTransmission = false);
    static void retransmitLastAckCallout();
    void resetTimeOuts();

    static void retransmitRequest();
    static void retransmitLastAckRequest();

    static void transmitAlarmRequest();
    enum alarmStatusCode {
        ReversCoordLostCon,
        CoordLostCon,
        ReversPeerLostCon,
        PeerLostCon,
        LastPeerLostCon,
        undefined
    };
    static enum alarmStatusCode classifyAlarmStatus();
    static void reactOnAlarmState(enum alarmStatusCode alarmState);
    void triggerReversRoute();
    static bool tripsGreaterThanLast(uint32_t compare);
    static void internalSetEnabled(bool status);
    void clearErrorFlags();
    static void incOrClearTimeout();

    // void handleSuspendRetransmit(AliveData& incomingAlive);
    friend class UnitSuiteTests;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_BGROUPFORWARDING_H_
