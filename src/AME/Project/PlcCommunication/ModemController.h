/*
 * ModemController.h
 *
 *  Created on: 19.09.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMCONTROLLER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMCONTROLLER_H_

#include <cstdint>

#include "../HelpStructures/BitMask.h"
#include "G3PlcSystem/PlcCommand.h"
#include "NetworkMember.h"
#include "../SignalProcessing/IoPinHandling/IoPin.h"
#include "../HelpStructures/Uint8Operations.h"
#include "../SystemBase/Global2.h"
#include "../SystemBase/TimeMeasurement.h"
#include "../HelpStructures/Node.h"
#include "../HelpStructures/List.h"
#include "../Terminal/SerialDrivers/SerialDriver.h"
#include "../Terminal/TermContextFilter.h"
#include "../SystemBase/Global.h"
#include "AliveHandling/NeighbourAssignment/AssignmentFlow.h"
#include "PlcCommandTypeSpecification.h"
#include "../UpdateModule/ITransmitter.h"

namespace AME_SRC {
// using namespace PLC_CommandTypeSpecification;

class ModemController : public ITransmitter {
 public:
    static enum displayState {
        displayNoMembers, displayMembersStart, displayMembersCont, displayIdle
    } actualDisplayState;
    typedef struct {
        uint16_t identifier;
        uint16_t tableIndex;
        const uint8_t *dataPtr;     // pointer to an binary array
        uint8_t dataLen;            // number of bytes in the binary array
    } getRequestArgs;

    static const uint8_t maxPANDescriptorSets = 5;  // Number of entries in panDescriptorContainer
    typedef union {
        struct {
            uint16_t panId;     // The PAN identifier as specified in the received beacon frame.
            uint16_t adress;    // The short address as specified in the received beacon frame
            uint16_t linkQuality;  // The LQI at which the network beacon was received Lower values represent lower LQI
            uint16_t rcCoord;   // The estimated route cost from LBA to the coordinator.
                                // It is used by the associating device to select LBA and PA
        };
        uint16_t discriptorItem[4];
    } PANDescriptor;
    // typedef struct{
    //  PlcCommand plcCmd;
    //  uint8_t plcNrCmd;
    // } plcCmdTupel;
    enum controllerStates {
        idle,
        init,
        singleCmd,
        newMember,
        routeDiscovery,
        adpGet,
        eapGet,
        eapSet,
        networkSearch,
        networkJoin,
        routing,
        dataTransmission,
        aliveRequest,
        getForceJoinInfoX,
        getForceJoinInfoLast,
        setForceJoinInfoX,
        setForceJoinInfoLast,
        getPLCAttributes,
        getPLCAttrLast,
        RemoteDiscoveryRequest,
        RemoteDiscoveryConfirm,
        aliveNeighbourParam,
        aliveNeighbourParaNext,
        forceJoinCoordDelPeer,
        sendDoubleBroadcast,
    };  // Add row by row to have GIT conformity
    enum createSetForceAttrMode { start, next, fc_only };   // modes for createSetForceJoinInfo
    static const uint8_t networkMemberMax = 30;
    enum eBGTransmitMode {bgIdle, manuell, processAuto};    // modes to transmit broadcast groups at coordinator
    enum eNsduContainerSize {kNsduContainerSize = 160};      // field size for one NSDU

 private:
    static bool receiveTokenAktiv;
    static IoPin *cpxResetPinPtr;
    static controllerStates controllerState;
    static bool notWaitingForConfirmation;
    bool newMemberJoined;
    uint16_t lastJoinedMemberIndex;
    static bool initFinish;
    static bool firmenwareLoaded;
    static bool connectedToCoordinator;
    static bool requestRepeatMode;
    static uint8_t nBGTransmitMode;  // mode to transmit broadcast groups at coordinator: element of eBGTransmitMode

    static bool remotePathReplyAwait;
    static bool aliveCheckEnabled;          // Alive-Check true=enabled, 0=blocked
    static uint16_t aliveCheckPresetLoops;  // Alive-Check with n loops up to end
    static uint16_t aliveCheckActLoops;     // Alive-Check with actually n loops up to end
    static uint8_t aktivNetMemberCount;
    static TimeMeasurement::measureJob *aliveTimeOutJobPtr;
    static BitMask<uint32_t> confirmMask;
    static Node<PlcCommand> *processCmdNodePtr;
    static Node<PlcCommand> *lastNodePtr;
    static uint8_t lastNodeNr;  // Consecutive number to internally distinguish a PlcCommand
    static List<PlcCommand> nodeCmdDefaultList;
    static uint8_t nodeStoreArrayIdx;
    static const uint8_t maxNodeStoreArrayIdx = 13;
    static Node<PlcCommand> nodeStorageArray[maxNodeStoreArrayIdx];
    static NetworkMember networkMemberList[networkMemberMax + 1];
    uint16_t networkMemberCount;
    static uint16_t panId;
//  static PlcCommand * getPlcCommandAndSetTargetLayerAndMessage(plcExecuteData* executeData);
    static PlcCommand& idleReturn(va_list funcArg);
    static PANDescriptor panDescriptorContainer[maxPANDescriptorSets];
    static uint8_t foundDescriptorAmount;   // Number of entries in panDescriptorContainer
    static uint8_t outgoingNsduContainer[kNsduContainerSize];

    // Replacement for Enum: Masks "New Members Mask" for variable newMemberRequest
    #define NMM_NONE    0
    #define NMM_ALIVE   1
    #define NMM_ALL     0xFF
    static int lastCheckIndex;
    static uint16_t aliveCheckInterval;
    static bool checkMemberPending;     // determines whether the next step in the alive check can be completed
    static uint8_t callOutCounter;
    static bool supressDisplayMessage;
    static int aliveTransmitIndex;
    static int aliveSendFailCount;
    static uint8_t newMemberRequest;
    static uint8_t memberListDisplayIndex;
    static TermContextFilter *termFilterPtr_;
    static int globalDestIP;    // target address for alive broadcast transmission
    AssignmentFlow &assignmentFlow_;

    void transformSequenzToPANDescriptorAndStoreAtContainerIndex(const uint8_t *sequenzPtr, uint8_t containerIndex);
    static void updateMemberIndexPendTime(uint16_t memberIndex);

 public:
    ModemController(IoPin *resetPin, SerialDriver *termPtr, AssignmentFlow &assignmentFlow);
    virtual ~ModemController();
    void setConfirmationPosition(uint8_t position);
    void transmitt(const uint8_t *message, uint8_t length, uint16_t adressIdentifer, char options[5]);
    void incrementProcessStateIfConfirmed();
    void triggerNewDeviceAuthentification(const PlcCommand &request, CHANNEL channelId);
    void updateAddressInfoWithJoinParameter(uint16_t ip, uint16_t panID);
    void activateCoordinatorWithJoinConfirmation();
    bool activateMemberFromJoinIndication(const PlcCommand &joinIndication);

    static void disableNetMemberAfterPendingTimeout();
    static void disableNetMemberAtIndex(uint16_t index, bool updateClientInfo, bool verbose = true);
    static void reTransmittInBuff();

    static void handleDisplayState();
    static void setSupressDisplayMessage(bool status) {
        supressDisplayMessage = status;
    }
    static bool isDisplayMessageSupressed() {
        return supressDisplayMessage;
    }

    static void updateAliveStatusWithReplyID(uint16_t id);
    static NetworkMember* findMemberWithShortID(uint16_t shortID);
    static uint8_t findMemberIndexWithShortID(int shortID);
    static char* showNetworkMembers(char *text);
    static void displayNetworkList();
    static void triggerSinglePlcRequestWithArgs(plcRequestNames requestName, int count, ...);
    static void triggerSinglePlcRequestWithArgs(PlcCommand &plcCmd);
    static void triggerRouteDiscoveryForChannelAndNetId(CHANNEL channelId, const uint16_t NetId);

    static void triggerSendDataWithMessageToNetIPAndRoutingMode(char *message,
            uint16_t netIP, bool routingMode);
    static void triggerSendDataWithLengthToNetIPAndRoutingMode(
            const uint8_t *message, uint8_t length, uint16_t netIP, bool routingMode);
    static void triggerSendBroadCastDataWithLengthToGroupeIP(
            const uint8_t *message, uint8_t length, uint16_t groupeIP, bool isImportant, uint32_t verboseShowMask);
    static void triggerSendDoubleBroadCastDataWithLengthToGroupeIPs(
            const uint8_t *message, uint8_t length, uint16_t groupeIP1, uint16_t groupeIP2);

    static void registerDiscoveryData(const PlcCommand &discoveryDataPtr);
    static void storeDataAtNodeIndex(uint32_t index, char prefix, const uint8_t *data, uint8_t dataLength);

    static void resetDiscoverySearchResults();
    static void resetModem();
    static void updateStorageClientInfoByIP(uint16_t memberIP);
    static void updateStorageClientInfoByIdx(uint16_t memberIdx);

    void storePANDiscriptorContainerFromDiscoveryConfirmation(const PlcCommand& confirmation);
    static PANDescriptor* getbestPanDescriptorFromDescriptorContainer();
    static char* PANDescriptorContainerToString(char *destBuff, int outLen, uint8_t containerIndex);


    static char* monitorAliveStatusAndGetLog();
    static void checkNetworkMembers();
    PlcCommand* executeProcessStateAndGetAnswer();
    static void setProcessState(Node<PlcCommand> *ptr) {
        processCmdNodePtr = ptr;
        lastNodePtr = NULL;
        confirmMask.clear();
    }

    bool isNotWaitingForConfirmation() const {
        return notWaitingForConfirmation;
    }

    bool hasProcessState() {
        return processCmdNodePtr != NULL;
    }

    static bool isNewMemberRequest(uint8_t mask) {
        return (newMemberRequest & mask);
    }

    static uint16_t createIPv6udpFrame(uint8_t* nsdu, const uint8_t* msg, uint8_t msg_len,
            uint16_t dest_net_ip, bool broadCast);

    static List<PlcCommand>& createStartNetworkPlcReqLisForChannel(CHANNEL channelID);
    #if ALIVE_CHECK_LEVEL >= 2
    static List<PlcCommand>& createAliveRequest(CHANNEL channelID);
    #endif
    static List<PlcCommand>& createInitPlcReqListForChannel(CHANNEL channelID);
    static List<PlcCommand>& createGetForceJoinInfo(bool lStart);
    static List<PlcCommand>& createSetForceJoinInfo(enum createSetForceAttrMode nMode);
    static List<PlcCommand>& createAttributeDump(bool lStart);
    List<PlcCommand>& createSetNeighbourGroupIds(uint8_t startIndex);
    static List<PlcCommand>& createSetGroupsWithGivenTupels(bool lStart,
                                                            uint8_t startIndexAliveMap, uint8_t startIndexModem, uint8_t count,
                                                            const IRoutingManager::MemberIpTupel *tupelArray);
    static List<PlcCommand>& disableNotConnectedPeers();

    void triggerNeighbourGroupWithIPs(uint16_t incominIp, uint8_t memberId, uint16_t outGoingIp);

    static controllerStates getControllerState() {
        return controllerState;
    }
    static void setControllerState(controllerStates newState) {
        controllerState = newState;
    }

    uint16_t getLastJoinedMemberIndex() {
        return lastJoinedMemberIndex;
    }

    bool isNewMemberJoined() const {
        return newMemberJoined;
    }

    static NetworkMember* getNetMemberPtrByIndex(uint16_t index) {
        if (index <= networkMemberMax) {
            return &networkMemberList[index];
        } else {
            return NULL;
        }
    }

    static NetworkMember* getNetMemberPtrByIP(uint16_t targetIP) {
        uint16_t index = 0;
        while (networkMemberList[index].getNetworkIPAddress() != targetIP) {
            if (index++ > networkMemberMax) {
                return NULL;
            }
        }
        return &networkMemberList[index];
    }

    static int getNetMemberIdxByIP(uint16_t targetIP) {
        int index = 0;
        while (networkMemberList[index].getNetworkIPAddress() != targetIP) {
            if (++index > networkMemberMax) {
                return -1;
            }
        }
        return index;
    }

    static uint16_t getNetMemberIPByIdx(uint16_t index) {
        return networkMemberList[index].getNetworkIPAddress();
    }

    static bool isInitFinish() {
        return initFinish;
    }

    static void setInitFinish(bool value) {
        initFinish = value;
    }

    static uint16_t getMacShortAddress();
    static void setMacShortAddress(uint16_t macShortAddress);

    static uint16_t getNetworkIPAddress();
    static void setNetworkIPAddress(uint16_t ip);

    static void setAliveMemberByIdx(uint16_t index, bool set);
    static void setMemberErrorByIdx(uint16_t index, bool set);

    static uint16_t getPanId() {
        return panId;
    }

    static bool isFirmenwareLoaded() {
        return firmenwareLoaded;
    }

    static void setFirmenwareLoaded(bool val) {
        firmenwareLoaded = val;
    }

    static void setNotWaitingForConfirmation(bool status) {
        notWaitingForConfirmation = status;
    }

    static bool isConnectedToCoordinator() {
        return connectedToCoordinator;
    }

    static void setConnectedToCoordinator(bool status) {
        connectedToCoordinator = status;
    }

    static uint8_t getNetworkMemberMax() {
        return networkMemberMax;
    }


    static bool isRemotePathReplyAwait() {
        return remotePathReplyAwait;
    }


    static void setRemotePathReplyAwait(bool status) {
        remotePathReplyAwait = status;
    }
    NetworkMember*  findNetworkmemberWithMacAddress(const uint8_t* macAddress);
    static uint16_t findNetworkmemberPositionWithMacAddress(const uint8_t* macAddress);

    static uint16_t getNetMemberIndexForShortAddress(uint16_t targetShortAddress);
    static int findNextAktivMemberIndexFromPosition(int position);
    static uint8_t countActivNetworkMembers();
    static uint8_t getNextInactiveNetworkIdx();
    static void restoreMemberList();
    // static void checkAliveTimeOut();

    static bool isAliveCheckEnabled() {
        return aliveCheckEnabled;
    }

    static void setAliveCheckEnabled(bool status) {
        aliveCheckEnabled = status;
    }

    static void setAliveCheckInterval(uint16_t interval) {
        aliveCheckInterval = interval;
    }

    static uint16_t getAliveCheckInterval(void) {
        return aliveCheckInterval;
    }
    static void setAliveCheckLoops(uint16_t loops) {
        aliveCheckPresetLoops = loops;
        aliveCheckActLoops = loops;
    }
    static uint16_t getAliveCheckPresetLoops() {
        return aliveCheckPresetLoops;
    }
    static void setAliveCheckActLoops(uint16_t loops) {
        aliveCheckActLoops = loops;
        if (loops == 0) {
            setAliveCheckEnabled(0);
        }
    }
    static uint16_t getAliveCheckActLoops() {
        return aliveCheckActLoops;
    }

    static bool isNetworkMemberIndexAktiv(uint16_t id) {
        return networkMemberList[id].getLink() == NetworkMember::connected;
    }

    static bool isNetworkMemberIPAktiv(uint16_t ip) {
        NetworkMember *memberPtr = getNetMemberPtrByIP(ip);
        return memberPtr->getLink() == NetworkMember::connected;
    }

    static uint16_t getNumPeersConnected(bool checkOutgoingRoute);
    static uint16_t getNumPeersAliveAssigned();
    static uint16_t getNumPeersPending();
    static char* getListPeersPending(char *output, uint32_t maxLen);

    static void aktivateMemberWithIPAtIndex(uint16_t ip, uint16_t index, bool coordinatorMode) {
        NetworkMember &pn = networkMemberList[index];
        if (coordinatorMode) {
          pn.setNetworkPanAddress(panId);
          pn.adaptMacWithNetAddress();
        }
      pn.setNetworkIPAddress(ip);
        pn.setLink(NetworkMember::connected);
    }

    static int aktivateNextFreeMemberWithIP(uint16_t ip, bool coordinatorMode) {
        uint16_t index = 1;  // vorher 2!
        while (networkMemberList[index].getLink() == NetworkMember::connected) {
            index++;
        }
        if (index <= ModemController::getNetworkMemberMax()) {
            aktivateMemberWithIPAtIndex(ip, index, coordinatorMode);
            return static_cast<int>(index);
        }
        return -1;
    }

    static void finishInitNetworkMember();

    static void deActivateAllMembers();
    static void activateTestMembersUptoIP(uint16_t lastIP);
    static float calculateLinkQualityinDB(uint8_t LQIvalue);
    static const uint8_t* getMessagePtrFromPlcCommand(
            const PlcCommand &confirmationPtr);

    static TimeMeasurement::measureJob* getAliveTimeOutJob() {
        return aliveTimeOutJobPtr;
    }

    static void setAliveTimeOutJob(TimeMeasurement::measureJob *job) {
        aliveTimeOutJobPtr = job;
    }

    static uint8_t& getAliveCallOutCounterReff() {
        return callOutCounter;
    }

    static void setAliveCallOutCounter(uint8_t counter) {
        callOutCounter = counter;
    }

    static uint8_t getNodeStoreArrayIdx() {
        return nodeStoreArrayIdx;
    }
    static uint8_t getNextNodeStoreArrayIdx() {
        if (++nodeStoreArrayIdx >= maxNodeStoreArrayIdx) {
            nodeStoreArrayIdx = 0;
        }
        return getNodeStoreArrayIdx();
    }
    static Node<PlcCommand>& getNodeStoreArrayPtrByIdx(uint32_t index) {
        return nodeStorageArray[index];
    }

    static uint8_t getFoundDescriptorAmount() {
        return foundDescriptorAmount;
    }
    static PANDescriptor& getPanDescriptorContainerPtrByIdx(uint32_t index) {
        return panDescriptorContainer[index];
    }

    static List<PlcCommand>& getNodeCmdDefaultList() {
        return nodeCmdDefaultList;
    }

    static int getGlobalDestIp() {
        return globalDestIP;
    }

    static int setGlobalDestIp(int destIP) {
        return globalDestIP = destIP;
    }

    static bool isRequestRepeatMode() {
        return requestRepeatMode;
    }

    static void setRequestRepeatMode(bool mode) {
        requestRepeatMode = mode;
    }

    static uint8_t getBGtransmitMode() {
        return nBGTransmitMode;
    }

    static void setBGtransmitMode(eBGTransmitMode set) {
        nBGTransmitMode = (uint8_t) set;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_MODEMCONTROLLER_H_
