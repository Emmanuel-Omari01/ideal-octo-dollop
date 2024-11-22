/*
 * SimpleDiscoveryManager.h
 *
 *  Created on: 25.05.2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_SIMPLEDISCOVERYMANAGER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_SIMPLEDISCOVERYMANAGER_H_

#include "IDiscoveryManager.h"
#include "../../../SystemBase/ProgramConfig.h"
#include "../../ModemController.h"
#include "../../../Terminal/SerialDrivers/SerialDriver.h"
#include "../../../Terminal/TermContextFilter.h"

namespace AME_SRC  {

class SimpleDiscoveryManager : public IDiscoveryManager {
 public:
    explicit SimpleDiscoveryManager(SerialDriver *termDrivePtr);
    uint8_t getData(const char *cWhat);
    char* showData(const char *cWhat);
    bool isDataShown();
    void storeData(uint8_t status, const uint8_t *data, uint8_t length);
    char* triggerDiscovery(uint8_t select, const char *cParameter);
    virtual ~SimpleDiscoveryManager();

 private:
    enum eDisplayState {
        idle, start, pending
    };
    enum eActionState {
        idleAction,
        // coordinator
        manual,
        automatic,
        auto_next,
        show,
        // Peer
        waitPeerDiscovery,
    };
    enum eCoorPeerAckReceivedState {
        ackUnknown, ackPending, ackReceived,
    };
    enum eSendDiscoveryRequest2PeerMode {
        requestStart, requestData,
    };
    enum { allPeers = 0xFFFF };
    enum { defaultSearchTime = 20 };  // Default discovery time in seconds
    enum { defaultCallOuts = 4 };     // Number of repeats in case of error
    static TermContextFilter *termFilterPtr_;
    uint8_t displayState;             // state with an element of eDisplayState
    int8_t displayEntryN;             // index during output; -1 = display header
    uint8_t displayActIdx;            // array-Index of peerLqiArray, which is now displayed
    uint16_t displayPeer;             // Peer, who has to be displayed: <allPeers> for all

    // #pragma pack    // DS: pragma pack option works not only here, but also in other places
    typedef struct {
        uint16_t ipAdr;         // IP address from the beacon frame
        uint8_t  linkQuality;   // look at PANDescriptor. uint8_t is enough here
    } PeerLQI;
    // #pragma unpack
    static const uint8_t size_eff_peerLQI = 3;

    // For coordinator:
    // Structur to save the discovery results of an individual peer
    typedef struct {
        uint16_t peerIP;            // IP address of the peer concerned
        uint8_t  descriptorCounts;  // number of descriptors with link quality
        PeerLQI peerLqiArray[ModemController::maxPANDescriptorSets];
    } PeerSetLQI;
    static const uint16_t peerArrLQISize = ModemController::networkMemberMax + 1;
    static PeerSetLQI peerArrayLQI[peerArrLQISize];
    static uint8_t peerArrayEntriesUsed;    // 0..peerArrLQISize
    uint8_t discoveryState;     // state with an elemente of eDiscoveryState
    uint8_t actionState;        // element of eActionState
    uint8_t targetIP;
    uint8_t ignorePeerRxTimes;      // @Peer, test: ignore requests of coordinator for n times
    uint8_t ignorePeerTxTimes;      // @Peer: test: Ignore dispatch to coordinator n times
    uint8_t coordPeerAckReceived;   // element of eCoorPeerAckReceivedState
    uint8_t actCallNum;
    uint16_t timeCounter;
    uint16_t numDiscoveriesToGo;    // For displaying the participants who will be considered for discovery
    uint16_t numDiscoveriesDone;    // For displaying the participants who have already been examined
    BitMask<uint32_t> discoveryPendingMask;

    void wrapShowData(const char *cWhat);
    void clearAllDiscoveryData();
    void registerDiscoveryData(const uint8_t *data);
    bool checkSendDiscoveryData(const uint8_t *data);
    void triggerSendDiscoveryData(bool noErrors);
    void triggerPeerRemoteDiscoveryRequest(const uint8_t *data);
    PeerSetLQI* searchDiscoveryDataContainer(uint16_t ipAdr, bool newContainer);
    void showNextPeer();
    void sortPeerLQI();
    void peerLqiAdd(PeerSetLQI *pContainer, const PeerLQI &newSet);
    void releasePeerLQI(uint16_t containerIP);
    void  startAutoDiscovery(bool lRestart);
    void  nextAutoDiscovery(uint16_t parameter, bool ipOrIndex);
        bool checkDiscoveryResults();
    void unSetPendingDiscovery(uint16_t peerIP);
    char* terminalNetworkSearch(const char *text);
    char* getRSearchErrorMessage(uint16_t peerIP, uint8_t searchTime);
    char* sendDiscoveryRequest2Peer(uint8_t peerIP, uint16_t searchTime, eSendDiscoveryRequest2PeerMode mode);
    char* _50msActions();
    void timeoutRDSend2Peer();
    void timeoutRestart();
    bool debugOutputEnabled();
    /***
    void createTimeoutHandling();
        void peerDiscoveryTimeout();
        void peerDiscoveryCallout();
    ***/
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_SIMPLEDISCOVERYMANAGER_H_
