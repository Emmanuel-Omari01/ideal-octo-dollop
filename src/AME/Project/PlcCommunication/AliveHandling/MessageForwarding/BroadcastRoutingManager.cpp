/*
 * broadcastRoutingManager.cpp
 *
 *  Created on: Jul 14, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "BroadcastRoutingManager.h"

#include <cstdio>

#include "../../../SystemBase/Global2.h"

namespace AME_SRC {


BroadcastRoutingManager::BroadcastRoutingManager() :
        storeIndex_(0), searchValue_ { 0 } {
    for (int i = 0; i < maxTupelAmount; i++) {
        MemberIpTupel &tupel = memberIpMap_[i];
        tupel.ip = 0;
        tupel.meberID = notUsedID;
    }
}

BroadcastRoutingManager::~BroadcastRoutingManager() {
    // TODO(AM) Auto-generated destructor stub
}

// TODO(AME): GetData Very confusing. Better create a mapper class
uint16_t BroadcastRoutingManager::getData(enum IRoutingManager::dataKey dtaKey) {
    MemberIpTupel *pairPtr;
    enum { nextTupelAfterReceiver = 1 };
    switch (dtaKey) {
        case routingRowEntryIndex:
            return storeIndex_;
        case IRoutingManager::GET_MEMBER_ID_AT_INDEX:
            return memberIpMap_[storeIndex_].meberID;
        case IRoutingManager::GET_IP_AT_INDEX:
            return memberIpMap_[storeIndex_].ip;
        case IRoutingManager::GET_MEMBER_ID_AT_SEARCH_VAL:
            pairPtr = searchForValueByIdOrIp(searchValue_, Type_MemberIP);
            if (pairPtr) {
                return (uint8_t) pairPtr->meberID;
            } else {
                return IRoutingManager::notUsedID;
            }
            break;
        case IRoutingManager::GET_IP_AT_SEARCH_VAL:
            pairPtr = searchForValueByIdOrIp(searchValue_, Type_MemberID);
            if (pairPtr) {
                return (uint8_t) pairPtr->ip;
            } else {
                return 0;
            }
            break;
        case SEARCH_TRANSMITTER_IP_BY_RECEIVER_IP:
        case SEARCH_TRANSMITTER_IP_BY_LISTENER_IP:
        case SEARCH_IP_FOR_SAME_ROLE_IN_FOLLOWING_SECTION:
            return getIpForSearchRequest(dtaKey);
            break;
        default:
            break;
    }
    return 0;
}

uint16_t BroadcastRoutingManager::getIpForSearchRequest(enum IRoutingManager::dataKey dtaKey) {
    enum {nextTupelAfterReceiver = 1, neglistenerOffset = 2, sectionOffset = 5};
    int offset = 0;
    switch (dtaKey) {
        case SEARCH_TRANSMITTER_IP_BY_RECEIVER_IP:
            offset = nextTupelAfterReceiver;
            break;
        case SEARCH_TRANSMITTER_IP_BY_LISTENER_IP:
            offset = -neglistenerOffset;
            break;
        case SEARCH_IP_FOR_SAME_ROLE_IN_FOLLOWING_SECTION:
            offset = sectionOffset;
            break;
        default:
            return 0;
            break;
    }
    return getIpFromSearchValueWithOffset(offset);
}


uint16_t BroadcastRoutingManager::getIpFromSearchValueWithOffset(int offset) {
    MemberIpTupel *startTupel = searchForValueByIdOrIp(searchValue_, Type_MemberIP);
    if (startTupel) {
        if (((startTupel - memberIpMap_) + offset) < maxTupelAmount) {
            return (uint8_t) startTupel[offset].ip;
        }
    }
    return 0;
}



IRoutingManager::MemberIpTupel* BroadcastRoutingManager::searchForValueByIdOrIp(
        uint16_t value, bool type) {
    for (int memberIdx = 0; memberIdx < IRoutingManager::maxTupelAmount;
            memberIdx++) {
        if (type == Type_MemberIP) {
            if (memberIpMap_[memberIdx].ip == value) {
                return &memberIpMap_[memberIdx];
            }
        } else {
            if (memberIpMap_[memberIdx].meberID == value) {
                return &memberIpMap_[memberIdx];
            }
        }
    }
    return 0;
}

#pragma GCC diagnostic ignored "-Wstack-usage="
char* BroadcastRoutingManager::showData(__attribute__((unused)) const char *cWhat) {
    uint8_t nameAmount = 7;
    const char roleNames[nameAmount][12] = { { "Receiver" }, { "Transmitter" },
            { "Supervisor" }, { "ListenerF1" }, { "ListenerB1" }, { "Admin" }, { "Unknown" } };
    const char *selectedNamePtr = 0;
    int offset = 26;
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "| RoleName|RoleID| IP|\n");
    snprintf(Global2::OutBuff + offset, Global2::outBuffMaxLength, "+-----------+-------+---+\n");
    for (int memberIdx = 0; memberIdx < IRoutingManager::maxTupelAmount; memberIdx++) {
        MemberIpTupel &selectedTupel = memberIpMap_[memberIdx];
        switch (selectedTupel.meberID) {
            case IRoutingManager::Receiver:
                selectedNamePtr = roleNames[0];
                break;
            case IRoutingManager::Transmitter:
                selectedNamePtr = roleNames[1];
                break;
            case IRoutingManager::Supervisor:
                selectedNamePtr = roleNames[2];
                break;
            case IRoutingManager::ListenerF1:
                selectedNamePtr = roleNames[3];
                break;
            case IRoutingManager::ListenerB1:
                selectedNamePtr = roleNames[4];
                break;
            case IRoutingManager::Administrator:
                selectedNamePtr = roleNames[5];
                break;
            default:
                selectedNamePtr = roleNames[6];
                break;
        }
        offset = (26 * (memberIdx + 2));
        if (offset + 26 < Global2::outBuffMaxLength && selectedTupel.ip != 0) {
            snprintf(Global2::OutBuff + offset, Global2::outBuffMaxLength, "|%11s|%7d|%3d|\n",
                    selectedNamePtr, selectedTupel.meberID, selectedTupel.ip);
        }
    }
    *(Global2::OutBuff + (offset + 1)) = 0;
    return Global2::OutBuff;
}

bool BroadcastRoutingManager::isDataShown() {
    return 0;
}

void BroadcastRoutingManager::storeData(IRoutingManager::dataKey key, uint16_t value) {
    switch (key) {
        case IRoutingManager::routingRowEntryIndex:
            storeIndex_ = value;
            break;
        case IRoutingManager::SEARCH_Value:
            searchValue_ = value;
            break;
        case IRoutingManager::GET_IP_AT_INDEX:
            memberIpMap_[storeIndex_].ip = value;
            break;
        case IRoutingManager::GET_MEMBER_ID_AT_INDEX:
            memberIpMap_[storeIndex_].meberID = value;
            break;
        default:
            break;
    }
}

}  // namespace AME_SRC
