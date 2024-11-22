/*
 * jumpMapper.h
 *
 *  Created on: Mar 6, 2024
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_JUMPMAPPER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_JUMPMAPPER_H_

#include <cstdint>

#include "../../../CompositeComponents/FoundationModule/ITimeOutWatcher.h"
#include "../../../Terminal/SerialDrivers/SerialDriver.h"
#include "../NeighbourAssignment/AssignmentFlow.h"

namespace AME_SRC {

class JumpMapper {
 public:
    JumpMapper(ITimeOutWatcher *watcherPtr, SerialDriver *notifyChannel, AssignmentFlow &assignFlowRef);
    virtual ~JumpMapper();
    void reactOnJumpRequest(uint16_t destGroupID, uint16_t roundtripID);
    static void reactOnAliveNoResponseRequest(uint16_t errorReporterIp,
            uint8_t roundTripID);
    static void setMappingByReporter(uint16_t errorReporterIp, uint8_t roundTripID);
    static void prepareNextRoundTrip();

    void transmitJumpRequestAndInform();
    bool isErrorReporterRequestValid(uint16_t errorReporterIp, uint8_t tripID);
    void informUserAboutJumpInvalidity();
    static void informUserAboutLostIPByReporter();

    void reply2NoResponseRequestReporter(uint16_t errorReporterIp);
    void reactOnJumpReply();
    void transmitJumpReply();

    bool isReporterLastInRoute() {
        return !isNotLastInRoute_ && isNoDoubledRequest_;
    }

 private:
    typedef struct {
        uint16_t targetIP;
        uint16_t targetRoutingIndex;
        uint16_t calloutIndex;
        uint16_t bcIP;          // IP address to which the message is send via broadcast
        uint16_t reporterIP;    // initially Peer-# who has recognized the loss.
                                // Variable may be changed by the coordinator if it detects further failures
        uint16_t triggerIP;     // Peer-# who has recognized the loss.
                                // Variable is used to block re-entries.
        uint8_t roundTripID;    // Peer with triggerIP send this round trip number.
                                // Variable is used to block re-entries.
        bool calloutHandling;
    } jumpMapping;

    static void transmittAliveJumpRequest();
    static void transmittAliveJumpMessageContainer();
    static void aliveJumpRequestCallout();

    static char aliveJumpMessageContainer_[];   // BGroupForwarding::AliveMsgSize
    static jumpMapping nextJumpMapping_;
    static AssignmentFlow *assignFlowPtr_;

    void forwardAliveJump2GroupWithID(uint16_t groupID, uint8_t roundtripID);
    bool isTimeOutIDValidAndRunning(uint8_t timeOutID);
    void informAboutTimeOutInvalidity(uint8_t timeOutID);

    static ITimeOutWatcher *watcher_;
    static TermContextFilter *termFilterPtr_;
    static uint16_t lostIp_;

    ITimeOutWatcher::timeOutInfo *aliveJumpRequestTOut_;
    ITimeOutWatcher::timeOutInfo *repeatJumpTimeOut_;
    enum timeOutStates {
        notInitialized, running, idle
    } timeOutState_;

    bool isNotLastInRoute_;
    bool isNoOtherJumpRequest_;
    bool isNoDoubledRequest_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_JUMPMAPPER_H_
