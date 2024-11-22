/*
 * AssignmentFlow.h
 *
 *  Created on: Aug 8, 2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ASSIGNMENTFLOW_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ASSIGNMENTFLOW_H_

#include <cstdint>

#include "AdjazenzParent.h"
#include "../../../Terminal/SerialDrivers/SerialDriver.h"
#include "../MessageForwarding/IRoutingManager.h"
#include "../../../SignalProcessing/Aktorik/LED/LEDSignalFlow.h"
#include "../../../Terminal/TermContextFilter.h"

namespace AME_SRC {

class AssignmentFlow {
 public:
    AssignmentFlow(SerialDriver &serialDriv, LEDSignalFlow &ledFlow);
    virtual ~AssignmentFlow();

    enum eFlowStep {
        idle,
        waitAutomaticA,
        waitAutomaticB,
        OuterAutomaticStart,
        OuterAutomaticLoop,
        routeDiscovery,
        buildTree,
        proofTree,
        sendBCOrder,
        startAlive
    };
    enum eCoordinatorRoutingStep {
        idleRouting, start, waiting, checkPeerNum
    };
    enum eRemoteDiscoveryStep {
        idleRD, startRemoteDiscovery, waitRD, showRD, loopRD
    };
    enum eTreeCreateStep {
        idleTree,
        startTree,
        showTreeStart,
        showTreeNext,
        showRouteStart,
        shortenTree,
        showShortenedTree,
        finishedTree
    };
    enum eTreeProofStep {
        idleTProof, treeProof, treeProofDecision
    };
    enum eBGAssignStep {
        idleAssign, startAssign, loopAssign, nextAssign
    };
    enum eAliveStep {
        idleAlive, startAliveSub, waitAlive, triggerAlive
    };
    enum eCallCause {
        timerBase, plcMsg, refreshOuterTimeOut
    };  // calling causes for processAssignment
    enum eIsMemberIndex {
        max, isMember, notConnected
    };

    enum {
        plcBGStartOffset = 0
    };  // offset/location for the first broadcast group at peer/coordinator (max. 16 groups)
    // enum eFillTupelArray { noEntries = -1, noPeerEntries = -2 };     // neg. value for fillTupelArray
    enum eMaxTupel {
        maxTupelQuantity = 50
    };  // max. number of tuples at coordinator in the array for broadcast groups
      // compare with ModemInterpreter, tupelArray
    enum eTupelContexts {
        create, transfer, maxContexts
    };
    typedef struct {
        IRoutingManager::MemberIpTupel *pTupelArray;
        uint8_t numTupelEntries;
        uint8_t ofsRegModem;
        uint8_t ofsAliveMap;
    } TupelContext;

    // Coordinator Part
    const char* processAssignment(enum eCallCause cause);
    uint16_t getRoutingEntry();
    void setRoutingEntry(uint16_t);
    // Peer Part
    void addNeighbourTupels(const uint8_t modemRegOffset,
            const uint8_t aliveMapOffset, const uint8_t count,
            const IRoutingManager::MemberIpTupel *tupelArray);

    static AdjazenzParent* getAdjazenzParent() {
        return adjazenzParentPtr_;
    }

    // Testsuite Interface
    static bool getData(const char *cWhat, char *cOutput, size_t outputLen,
            const char *cFormat, int count, ...);
    static bool setData(const char *cWhat, int count, ...);
    static bool runFunction(const char *cWhat);

    // Terminal commands
    static char* triggerAutomaticSteps(char *text);
    static char* fillAdjMatrixTerminal(char *text);
    static char* triggerShowNetworkDiscoveryResults(char *text);
    static char* triggerNetworkDiscovery(uint8_t select, char *text);
    static char* remoteBroadcastGroupSelection(char *text);
    static char* broadcastGroupSelection(char *text);
    static char* terminalSetFlowSteps(char *text);
    static void setFlowStep(uint8_t step, char *caller);
    static void setFlowSubStep(uint8_t step, char *caller);
    static void setFlowSteps(uint8_t mainStep, uint8_t subStep, char *caller);
    static void showFlowSteps(bool showAlways, uint32_t mask, char *caller);
    static void clearAllFlowTimeouts();

    static uint16_t getNextRouteIpAfterIp(uint16_t srcIP);

    static uint16_t getRoutingIndexForIp(uint16_t targetIp);
    static uint16_t getRoutingIPForIndex(uint16_t index);

    static void setAutomaticMode(bool mode) {
        automaticMode = mode;
    }
    static void setNumMinPeers(uint8_t quantity) {
        numMinPeers = quantity;
    }
    static void coordRoutingCallBack(uint8_t status);
    static void incFlowStep();
    static void refreshRoutingDelay();

    static void setPeerIgnoreAssignRequests(uint8_t num) {
        peerIgnoreAssignRequests = num;
    }
    static uint8_t getPeerIgnoreAssignRequests() {
        return peerIgnoreAssignRequests;
    }
    static void showCoordinatorVerboseBarExpectedTime(uint8_t expectedTime);
    static TupelContext* getTupelContext(uint32_t conTextIdx);
    static void releaseTupelContext(uint32_t conTextIdx);

    bool isLastRoutingIP(uint16_t srcIP);
    bool isPreLastRoutingIP(uint16_t srcIP);
    static char* showMatrixOnTerminal(char *text);
    static char* presetAliveRoute(char *text);
    static void handlePeerAssigned(uint16_t peerIP);

 private:
    enum eDoAssignMode {
        edaStart, edaPreCalculation, edaNext, edaRepeat
    };
    // Or-Masks for the state of this class
    enum eStatus {
        none,
        dwellTimeCompleted = 1,
        routed = 2,
        remoteDiscovered = 4,
        treeCreated = 8,
        routeAssigned = 0x10,
        aliveStarted = 0x20,
    };
    // modes for function setAutomaticState
    enum eStatusUpdate {
        set, clear, overwrite
    };

    typedef struct {
        uint16_t currentIP;
        IRoutingManager::MemberIpTupel *memberRoles;
        uint16_t rolesOffset;
        uint8_t roleAssignmentLevel;
        uint8_t roleAmount;
        uint16_t transmitterGroup;
        uint8_t routeOrderIdx;
    } MemberRolesProcessContext;
    LEDSignalFlow &ledFlowRef_;

    static bool doAssignment(enum eDoAssignMode mode);
    static void storeDataForBypassingEOBIntersection();
    static void printEOBBypassingResults();
    static void createBGSingleTransmission(uint8_t tIP,
            IRoutingManager::MemberIpTupel *pTupelArray, uint8_t modemRegOffset,
            uint8_t aliveMappingOffset, uint8_t count);
    static void createTransMessageFromTupels(char *messageContainer,
            IRoutingManager::MemberIpTupel *pTupelArray, uint8_t count);
    static int16_t storeTupelsByEndingPrediction(
            IRoutingManager::MemberIpTupel *tupelsPtr, uint16_t peerIndex,
            AdjazenzParent::tuple *pTreeTupel);
    static void fillTupelTripelWithDefaultRoles(
            IRoutingManager::MemberIpTupel *tupelsPtr);
    static void fillTupelTripelWithBranchEndingRoles(
            IRoutingManager::MemberIpTupel *tupelsPtr);
    static void storeTupels(uint8_t length,
            IRoutingManager::MemberIpTupel *tupelsPtr);
    static void insertAdminRoleInTupel(
            IRoutingManager::MemberIpTupel *tupelsPtr);
    static void transformOrderPartToRoleBroadcastTupels(
            AdjazenzParent::tuple *treeNavigationOrder);
    static void fillPeerOrderPositionArray(uint16_t peerIP,
            uint8_t *positionArray, uint8_t size);
    static IRoutingManager::MemberIpTupel* getCommunicationRolesByPositionAndIp(
            uint8_t *positionArray, uint16_t peerIP);
    static void fillMemberRollsForProcessContext(
            MemberRolesProcessContext &rolesContext);
    static uint8_t* getRolesForProcessContext(
            MemberRolesProcessContext &processContext);
    static void mapRolesToGroupIpsForProcessContext(uint8_t *roles,
            MemberRolesProcessContext &rolesContext);
    static uint16_t determineIpForRoleInProcessContext(uint8_t targetRole,
            uint8_t roleOffset, MemberRolesProcessContext &processContext);
    static void solveDoubleRolesForProcessContext(
            MemberRolesProcessContext &processContext);
    static void clearListenerRolesForEOB(
            MemberRolesProcessContext &processContext, uint8_t *positionArray);
    static IRoutingManager::MemberIpTupel* getCommunicationRolesForPeerIP(
            uint16_t peerIP);
    static void editeRolesIfPeerIPAtRoundTripEnd(
            IRoutingManager::MemberIpTupel *comRoles, uint16_t peerIP);
    static uint8_t getCommunicationRoleAmountFromRoles(
            IRoutingManager::MemberIpTupel *pTupelArray);
    static bool isTupelCapable(IRoutingManager::MemberIpTupel *pt);
    static uint8_t removeNotCapableTupels(uint8_t comRoleAmount,
            IRoutingManager::MemberIpTupel *comRoles);
    static void transferCommunicationRolesToIP(
            IRoutingManager::MemberIpTupel *comRoles, uint16_t targetIP);
    static void printTransferInformation(uint8_t comRoleAmount,
            IRoutingManager::MemberIpTupel *comRoles, uint16_t targetIP);
    static int isMemberIndexCoordinatorOrConnected(uint8_t netIndex);
    static void restoreNeighbourTupels();

    static const char* startRoutingDelay();
    static void timeoutRoutingStart();
    static void doStartOuterTimeoutControl();
    static void TimeOutOuterControl();
    static const char* doRouteDiscovery(enum eCallCause cause);
    static void createRDTimeoutHandling();
    static void peerRouteDiscoveryTimeout();
    static void peerRouteDiscoveryCallout();

    static const char* doBuildTree();
    static const char* doProofTree();
    static const char* doAssignAutomatic(enum eCallCause cause);
    static const char* doStartAssign();
    static void assignmentTimeout();
    static void assignmentCallOut();
    const char* doStartAlive();

    static void setAutomaticState(eStatus neu, eStatusUpdate mode);
    static bool testAutomaticState(eStatus test);

    static TermContextFilter *termFilterPtr_;
    static AdjazenzParent *adjazenzParentPtr_;
    static bool processing;     // 0..AdjazenzParent::getMaxSource()
    // static uint16_t ipCounter;

    static uint8_t flowStep;        // enum eFlowStep
    static uint8_t flowSubStep;     // enum eCoordinatorRoutingStep,
    static bool automaticMode;      // false: manual
    static uint8_t automaticStatus;  // {eStatus}
    enum {
        unknown, PeerRouting, PeerCheck
    };
    static uint8_t routingMode;
    static uint8_t numMinPeers;
    static uint8_t cmpAssignedPeers;
    static uint8_t assignLoopCallOuts;
    static uint8_t assignPeerIdx;  // actual peer index in the network member list in the broadcast assignment process
    static uint8_t peerIgnoreAssignRequests;
//  static IRoutingManager::MemberIpTupel tupelCoordArray[maxTupelQuantity];
    // => if necessary locate array to heap or stack
    static TupelContext tmpTupelContext[maxContexts];

    typedef struct {
        uint16_t transmitterIP;
        uint16_t transmitterGroup;
        uint16_t receiverIP;
        uint16_t receiverGroup;
    } intersectionBypassInfo;
    static intersectionBypassInfo routeBypassInfo[10];
    static uint16_t transmissionIPBeforeEOB;

    friend class UnitSuiteTests;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ASSIGNMENTFLOW_H_
