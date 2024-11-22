/*
 * AutostartFlow.h
 *
 *  Created on: 14.02.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMLOGIC_AUTOSTARTFLOW_H_
#define SRC_AME_PROJECT_SYSTEMLOGIC_AUTOSTARTFLOW_H_

#include "../CompositeComponents/FoundationModule/IRandGenerator.h"
#include "../PlcCommunication/ModemInterpreter.h"
#include "../SignalProcessing/Aktorik/LED/LEDSignalFlow.h"
#include "../Terminal/SerialDrivers/SerialDriver.h"
#include "../Terminal/TermContextFilter.h"
#include "../Test/UnitSuite/UnitSuiteTests.h"

namespace AME_SRC {

// #define USE_preset_Search_Mode   // determines whether the AutostartFlow step chain runs again
                                    // In the first pass usually the force join process is practiced
                                    // In the second pass (if force join fails) the default search is practiced.
                                    // Currently switched off because after a failed force join
                                    // the default search is selected and then a system reset is established
class AutostartFlow {
 public:
    typedef enum {
        initMsg,
        resetModemStep,
        waitBootloaderStep,
        initModemStep,
        waitInitConfirmStep,
        checkForceJoinTabStep,
        // startupReloadAndJoin()
        startupReloadStep,
        waitReloadStep,
        ReloadJoinStep,
        ReloadJoinConfirmStep,
        PeerFJProofJoinStep,
        // Peer
        ReloadWaitRandomStep,
        ReloadSendP2CVerification1a,
        ReloadSendP2CVerification1b,
        ReloadSendP2CVerification2a,
        ReloadSendP2CVerification2b,
        ReloadSendP2CVerification3a,
        ReloadSendP2CVerification3b,
        ReloadWaitTimeout,
        // Coordinator
        ReloadCoordVerifyStart,
        ReloadCoordVerifyLoopStart,
        ReloadCoordVerifySendPeer,
        ReloadCoordInitDelay,
        ReloadCoordWaitPeerDelay,
        ReloadCoordVerifyLoopNext,
        // startupSearchAndJoin()
        searchNetworkStep,
        waitTillNextSearchStep,
        proofSearchResultsStep,
        joinNetworkStep,
        waitJoinConfirmStep,
        proofConnectionStep,
        connectionProofedStep,
        storeJoinInfoStep,
        waitAfterJoinFailStep,
        deleteNetworkAgentStep,
        routeOnce,
        waitRouting,
        waitSystemRestart,
        announceFinishStep,
        runTests,
        deactivated
    } autostartStep;
    typedef enum {
        plcInit, forceJoining, searching, done,
    } tPlcFlowState;

 private:
    static autostartStep bootAutoStart;
    static autostartStep plcAutoStart;
#ifdef USE_preset_Search_Mode
    static bool presetSearchMode;
#endif
    static bool proceed_again;
    static TermContextFilter *termFilterPtr_;
    ModemInterpreter &modemInterpreterReff;
    ModemController::PANDescriptor *bestFoundDescriptorPtr;
    char tmp[20];
    bool autostartFinishedStatus;
    IRandGenerator *randomGen;
    // Force Join check at coordinator
    static uint8_t CheckNextPeerIdx;
    static uint8_t CheckCallOuts;
    static int16_t CheckDelay;

    // Force Join check at peer
    static uint16_t FJPeerActTime;  // actually time [s]. FJPeerActTime > FJPeerTimeout: yes: timeout.
    static uint8_t FJPeerCounterJ;  // Counter to create a slowing waiting time for the next sending
                                    // of a verification request
    static uint8_t plcFlowState;    // {tPlcFlowState} To check if module is performing a network search or a force join
    static uint8_t ignoreNTimesFJRequests;  // Coordinator, Peer:
                                            // ignore force join requests on the counterpart for n times
    LEDSignalFlow &ledFlowReff_;

 public:
    AutostartFlow(SerialDriver *newSerialDriver,
            ModemInterpreter &newModemInterpreterReff, IRandGenerator *rG,
            LEDSignalFlow &ledFlow);
    virtual ~AutostartFlow();
    bool isFinished() {
        return autostartFinishedStatus;
    }

    static char* exitAutoStart(char *dummy);
    static char* setFJIgnoreRequests(char *text);
    static void setIgnoreNTimesFJRequests(uint8_t n) {
        ignoreNTimesFJRequests = n;
    }
    static uint8_t getIgnoreNTimesFJRequests() {
        return ignoreNTimesFJRequests;
    }

    void boot();
    void proceed();
    void startupReloadAndJoin();
    void startupSearchAndJoin();

    autostartStep getPlcAutoStart() const {
        return plcAutoStart;
    }
    void restartTestSuite();
    tPlcFlowState getPlcFlowState() const {
        return (tPlcFlowState) plcFlowState;
    }

 private:
    static void resetModem();
    static void resetSoftwareForDefaultSearch();

    void waitBootloaderIfNecessary();
    void evaluateBootloaderTimeOuts();
    void evaluateBootTimeOut();
    void evaluateResetTimeOut();

    void initModem();
    void setModemProfileAndStartTimeMeasurement();
    void initPLCModem();

    void waitInitConfirm(autostartStep startupStep);
    void evaluateInitResetTimeOut();

    bool checkForceJoinTabs();
    void checkForceJoinCoordinator();

    void searchNetwork();

    void waitTillNextSearch();
    void waitRandomTillJoin(IRandGenerator *randGen);

    void proofSearchResults();
    void evaluateSearchTimeout();

    void joinNetwork();
    void doConnectedAction();
    void forceJoinNetwork(autostartStep next);
    void ReloadSendP2CVerificationMsg();
    bool ReloadWaitP2CVerification();
    bool ReloadSendC2PVerificationMsg(uint16_t peerIdx);
    //  void ReloadWaitC2PVerification();
    bool ReloadCoordCheckPeers();
    void ReloadCoordDelPeers();

    bool waitJoinConfirm(autostartStep next, uint8_t joinConfirmWaitTime);

    void profJoinConnection(autostartStep joinStep, autostartStep retryStep,
            autostartStep failStep);

    void waitAfterJoinFail();

    void announceFinish();

    void deleteNetworkAgent();
    void FlowMode1AndResetProcess();

    bool getTimeOutStatusAndSetFlowStep(uint16_t waitTime,
            autostartStep transitionStep);
    void set_proceed_again() {
        proceed_again = true;
    }
    void setPlcFlowState(tPlcFlowState state) const {
        plcFlowState = (uint8_t) state;
    }

    friend class UnitSuiteTests;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMLOGIC_AUTOSTARTFLOW_H_
