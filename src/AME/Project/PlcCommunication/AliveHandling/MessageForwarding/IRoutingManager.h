/*
 * IRoutingManager.h
 *
 *  Created on: Jul 14, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_IROUTINGMANAGER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_IROUTINGMANAGER_H_

#include <cstdint>

namespace AME_SRC {

class IRoutingManager {  // @suppress("Class has a virtual method and non-virtual destructor")
 public:
    // IDiscoveryManager();
    enum dataKey {
        routingRowEntryIndex,
        ALL_ROUTES,
        SEARCH_Value,
        SEARCH_TRANSMITTER_IP_BY_RECEIVER_IP,
        SEARCH_TRANSMITTER_IP_BY_LISTENER_IP,
        SEARCH_IP_FOR_SAME_ROLE_IN_FOLLOWING_SECTION,
        GET_IP_AT_INDEX,
        GET_MEMBER_ID_AT_INDEX,
        GET_IP_AT_SEARCH_VAL,
        GET_MEMBER_ID_AT_SEARCH_VAL,
    };
    typedef struct {
        uint8_t meberID;
        uint16_t ip;
    } MemberIpTupel;
    enum { maxTupelAmount = 10 };
    enum eBCGroups {BCAliveIP = 0xF0, BCGlobalIP = 0xFF};
    enum eBGStartEnd { BGRoundStart = 0x01, BGStart = 0xFF, BGEnd = 0xFE, BGEndAck = 0xFD,
                       BGAdmin = 0x87, BGNotUsed = 0};
    enum eTupelRoles { ListenerF1 = 2, Transmitter = 1, Receiver = 0, Supervisor = 0xFF, ListenerB1 = 0xFE,
                        EndTransmitter = 0x7F, Administrator = 0x87, notUsedID = 0x70};
//  enum eTupelRolPosition  { ReceiverPos = 0 , TransmitterPos = 1 , SupervisorPos = 2};
    virtual uint16_t getData(enum dataKey) = 0;
    virtual char* showData(const char *cWhat) = 0;
    virtual bool isDataShown() = 0;
    virtual void storeData(enum dataKey, uint16_t value) = 0;
    // virtual ~IDiscoveryManager();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_IROUTINGMANAGER_H_
