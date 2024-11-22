/*
 * ModemInterpreter.h
 *
 *  Created on: 25.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMINTERPRETER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMINTERPRETER_H_

#include <cstdint>

#include "../SystemBase/Global2.h"
#include "G3PlcSystem/PlcCommand.h"
#include "../Terminal/TimedSerialDriver.h"

#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
#include "BootLoader/ModemBootLoader.h"
#endif

#include "../HelpStructures/List.h"
#include "NetworkMember.h"
#include "../SignalProcessing/IoPinHandling/IoPin.h"
#include "ModemController.h"
#include "ModemTransmitter.h"
#include "IoCommandManager.h"
#include "AliveHandling/MessageForwarding/JumpMapper.h"
#include "AliveHandling/NeighbourAssignment/IDiscoveryManager.h"
#include "../CompositeComponents/FoundationModule/ITimeOutWatcher.h"
#include "../SignalProcessing/Aktorik/LED/LEDSignalFlow.h"
#include "../Terminal/TermContextFilter.h"
namespace AME_SRC {

class ModemInterpreter {
 public:
    enum InterpretState {
        idle,
        aliveCheckRequest,
        repeatConnectionAnswer,
        switchMatrixRequest,
        switchMatrixSwitch,
        switchMatrixAnswer,
        announceUpdate,
        waitAnnounceAnswer,
        // startRouteUpdate,
        // waitRouteUpdate,
        // waitTillNextRoutingCall,
        updateMemberRouting,
        waitMemberRouting,
        showMembers,
        proofForceJoinVerification,
        proofDiscoveryTimeout,
        updateCIFlashStart,
        updateCIFlashNext,
        triggerBGroupSet,
        waitBGroupSet,
        searchFCEntry,
        evaluateFCResponse,
        waitBGAutoGroupSet,
        processBGAutoGroupSet,
        aliveReEntryMessage,
        aliveSendReEntryReply,
    };

    enum eForceJoinState {  // Masks:
        FJinit = 0,         // value after program start = undefined
        FJpending = 1,      // will be set at peer or coordinator as soon as the proving request runs
        FJconfirmed = 2,    // will be set at peer or coordinator as soon as the acknowledge message
                            // from the proving device is received
        c2pBCReceived = 4,  // will be set at peer as seen as a broadcast request (foreign address) was recognized
        c2pIPReceived = 8,  // will be set at peer as seen as a broadcast request (own address) was recognized
        FJtimeout = 0x10,   // will be set if an error was recognized
        FJerror = 0x20
    };
    typedef uint8_t sForceJoinState;

 private:
    static IDiscoveryManager* discMangerPtr;
    static InterpretState interpretState;
    static uint8_t nextInterpretState;
    SerialDriver& modemInterface;
    static NetworkMember* pingTargetPtr;
    static ModemController* modemControll;
    static ModemTransmitter* transmitter;

    AssignmentFlow& assignFlowRef_;
#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
    static ModemBootLoader* modemBootloader;
#endif

    static bool isNetworkSearchTriggerd;
    static bool peerRoutingProcess;
    static uint16_t dataStatusCode;
    static bool dataStatusSet;              // determines whether dataStatusCode was set by a received message

    static bool softwareNotInStorage;
    static bool macFrameReadCompleteStatus;

    static uint16_t deviceTableCurrentValue;
    static uint16_t deviceTableSearchValue;
    static uint16_t lastSwitchMessagePanIP;
    static uint16_t aliveReEntryPair[2];    // Target IP / Src group index
    static uint16_t lastDataIndicationSrcIP_;
    static uint32_t roundTripTime;
    static uint32_t frameCounter;

    static enum routingState {
        routeSuccess,
        routeFailure,
        invalidParam,
        routeTimeout,
        processing,
        routeIdle
    } lastRoutingState;

    uint8_t switchMatrixRowIndex;
    uint8_t networkMemberCount;
    uint8_t aktivNetMemberCount;
    uint8_t lastMessageSourceId;
    uint8_t networkSearchDurationSeconds;
    uint8_t lastTargetRoutingPeer;

    static uint8_t modulID;
    static uint8_t responsArray[ModemController::networkMemberMax + 1];
    static bool isRouteFound;
    static bool connected;
    static sForceJoinState ForceJoinState;

    ModemInterpreter();


#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
    bool notUploading;
    void proofModemStatusAndLoadSoftware(char *modemAnswer, const PlcCommand *answerPtr);
    void loadModemFirmenware(char *modemBootAnswer);
    void loadFirmenwareToModem(char *modemAnswer, const PlcCommand *requestPtr);
    void displayBootSteps(BootCommandManager::CommandID commandID);
#endif

    void deactivateSuccessfulKickedMembers(const PlcCommand *requestPtr);

    void displayMessageSrcHeader(uint16_t srcIP);
    void displayMessageOnTerminal();
    void displaySignalQualityOnTerminal(const PlcCommand &requestPtr);
    void interpretIncommingMessage(const PlcCommand &requestPtr);

        void triggerOwnResetRequest();
            static void handleOwnResetRequest();

    bool hasWaitNTimes(uint32_t N);
    void distinguishPlcCommandAndInformController(const PlcCommand& confirmation);
        void distinguishConfirmationAndInformController(const PlcCommand& confirmation);
            void actOnConfirmationSuccess(LAYER targetLayer, uint8_t cmdID, const PlcCommand& cmd);
            void actOnConfirmationFailur(LAYER targetLayer, uint8_t cmdID, uint8_t cmdState, const PlcCommand& cmd);
        void distinguishIndicationAndInformController(const PlcCommand& confirmation);
            void storeSrcAdressFromDataIndication(const PlcCommand& confirmation);

    void interpretPathDiscoveryParameter(const uint8_t* discoveryParamPtr);

 public:
    enum commandLetters {
        AliveRequest = 'A',
        AliveStopRequest = 'a',
        AliveLastAckReply = 'B',
        CoordinatorRoutingRequest = 'C',
        CoordinatorRoutingReply = 'D',
        errorAliveNoResponseRequest = 'E',
        errorAliveNoResponseReply = 'e',
        ForceJoinCheckRequest = 'F',
        BroadCastGroupReq = 'G',
        BroadCastGroupReply = 'H',
        InputRequest = 'I',
        InputReply = 'i',
        SwitchCommandConfirmation = 'K',
        PeerLeaveRequest = 'L',
        SwitchMatrixRequest = 'M',
        switchMatrixReply = 'm',
        OutputRequest = 'O',
        outputReply = 'o',
        PollRequest = 'P',  // PathDiscoveryRequest will not be pursued further 15.02.2024
        pollReply = 'p',
        RoutingMessage = 'R',
        SearchRequest = 'S',
        TimeoutsGlobalRequest = 'T',
        JoinActionRequest = 'j',  // triggers optional actions at coordinator after a peer join
        GlobalResetRequest = 'X',
        aliveJumpRequest = 'Z',
        aliveJumpReply = 'z',
    };
    static const char commandPrefix[];
    static struct routingIndex {
        uint8_t MemberIndex;
        uint8_t UpdateIndex;
    } targetRouting;

    static TermContextFilter *termFilterPtr_;
    ITimeOutWatcher* watcher_;
    IoCommandManager& ioCmdManager_;
    LEDSignalFlow& ledFlowReff_;

    JumpMapper& aliveJumpMapper_;

    void updateRouteToNeighbours();
    void answerStepwiseOnIncommingMessages();
    // ModemInterpreter(SerialDriver& newDriverPtr,uint8_t targetModulID);
    ModemInterpreter(SerialDriver &newDriver, SerialDriver &notifyChannel,
            IoPin *resetPinPtr, uint8_t targetModulID,
            IDiscoveryManager *discoveryManager, ITimeOutWatcher *watcher,
            IoCommandManager &ioCommandManager, AssignmentFlow &assignFlowRef,
            LEDSignalFlow &ledFlowReff, JumpMapper &aliveJumpMapper);

    virtual ~ModemInterpreter();
    void interpretEvents();
    void handelRequests();
    void handelPLCRequests(const PlcCommand *requestPtr);
    void handelBootloading();
    void executePingPong();

    uint32_t getFrameCounter();


#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
    void setIsNotUploading(bool isNotUploading) {
        this->notUploading = isNotUploading;
    }

    bool isNotUploading() const {
        return notUploading;
    }
#endif
    bool isConnected() {
        return connected;;
    }

    static uint16_t getLastDataStatusCoce() {
        return dataStatusCode;
    }
    static bool getDataStatusSet() {
        return dataStatusSet;
    }
    static void setDataStatusSet(bool lSet) {
        dataStatusSet = lSet;
    }

    static bool isSoftwareNotInStorage() {
        return softwareNotInStorage;
    }

    bool isFrameCounterReadComplete() {
        return macFrameReadCompleteStatus;
    }

    uint8_t getLastMessageSourceId() const {
        return lastMessageSourceId;
    }

    void setLastMessageSourceId(uint8_t _lastMessageSourceId_) {
        this->lastMessageSourceId = _lastMessageSourceId_;
    }




    uint8_t getNetworkSearchTime() const {
        return networkSearchDurationSeconds;
    }

    void setNetworkSearchTime(uint8_t networkSearchTime) {
        this->networkSearchDurationSeconds = networkSearchTime;
    }

    bool IsRouteFound() const {
        return isRouteFound;
    }

    sForceJoinState getForceJoinState() const {
        return ForceJoinState;
    }
    void setForceJoinState(sForceJoinState nSet) {
        switch (nSet) {
            case FJpending:
            case FJconfirmed:
            case c2pBCReceived:
            case c2pIPReceived:
            case FJtimeout:
            case FJerror:
                 ForceJoinState |= nSet;
                 break;
            default:
                ForceJoinState = nSet;
        }
    }

    static uint32_t getRoundTripTime() {
        return roundTripTime;
    }

    uint8_t getAktivNetMemberCount() const {
        return aktivNetMemberCount;
    }

#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
    ModemBootLoader* getModemBootLoader() const {
        return modemBootloader;
    }
#endif

    void setMacFrameReadCompleteStatus(bool status) {
        macFrameReadCompleteStatus = status;
    }

    void setFrameCounter(uint32_t counter) {
        frameCounter = counter;
    }

    void setIsNetworkSearchTriggerd(bool _isNetworkSearchTriggerd_) {
        this->isNetworkSearchTriggerd = _isNetworkSearchTriggerd_;
    }

    bool isLastRoutingSuccessful() const {
        return lastRoutingState == routeSuccess;
    }

    bool isPeerRoutingProcess() const {
        return peerRoutingProcess;
    }
    static ModemTransmitter* getTransmitter() {
        return transmitter;
    }

    static const char* getCommandPrefix() {
        return commandPrefix;
    }

    static void setInterpretState(InterpretState state) {
        interpretState = state;
    }
    static void setNextInterpretState(InterpretState state) {
        nextInterpretState = state;
    }



    static ModemController* getModemControllPtr() {
        return modemControll;
    }

    static char* triggerNetworkDiscovery(uint8_t select, char *text);

    static IDiscoveryManager* getDiscManagerPtr() {
        return discMangerPtr;
    }

    AssignmentFlow& getAssigntmentFlowRef() const {
        return assignFlowRef_;
    }

    bool isRequestForThisModul(const char *request);

    static uint16_t getLastDataIndicationSrcIp() {
        return lastDataIndicationSrcIP_;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMINTERPRETER_H_
