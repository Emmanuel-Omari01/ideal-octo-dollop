/*
 * NetworkMember.cpp
 *
 *  Created on: 05.05.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "NetworkMember.h"

#include <cstring>
#include <cstdio>

#include "../HelpStructures/Uint8Operations.h"

namespace AME_SRC {

NetworkMember::NetworkMember(const uint8_t* macAddressPtr, uint16_t newNetworkIP)
: networkIPAddress(newNetworkIP), aliveTimeout(0),
  pendingTime(NetworkMember::timeNotPending), link(disconnected),
  outgoingRoute(unknown), testMember(false),
  aliveMember(false), aliveError(false) {
  fillMacAddress(macAddressPtr);
}

void NetworkMember::fillMacAddress(const uint8_t* macAddressPtr) {
    memcpy(macAddress, macAddressPtr, 8);
}
// ToDo(AME-Team) translate German comments in English


/*** Diese Methode vergleicht eine übergebene MAC-Adresse mit der MAC-Adresse des Netzwerkteilnehmers
 * @param suggestedMacAdress Zu prüfende MAC adresse
 * @return true = Mac Adressen stimmen überein
 */
bool NetworkMember::hasEqualMacAddress(const uint8_t suggestedMacAddress[8]) {
    for (unsigned int i=0; i < sizeof(macAddress); i++) {
       if (this->macAddress[i] != suggestedMacAddress[i]) {
          return false;
       }
    }
    return true;
}

char *NetworkMember::toString(char *dest, uint32_t destLen) {
    char tmp[30];
    char tstMember = (testMember)? '*':' ';
    int remainingLen = destLen;
    int tempLen;
    *dest = 0;
    if (remainingLen > 12) {
        snprintf(dest, destLen, "MacAdr:");
        remainingLen -= strlen(dest);
    }
    for (unsigned int i=0; i < sizeof(macAddress); i++) {
        snprintf(tmp, sizeof(tmp), "%02X", macAddress[i]);
        tempLen = strlen(tmp);
        if (tempLen < remainingLen) {
            remainingLen -= tempLen;
            strncat(dest, tmp, destLen);
        }
    }
    int nr = uint8Operations::sum2BytesFromLSB(macAddress+6);
    snprintf(tmp, sizeof(tmp), " M.Nr.:%03d", nr);
    tempLen = strlen(tmp);
    if (tempLen < remainingLen) {
        remainingLen -= tempLen;
        strncat(dest, tmp, destLen);
    }

    snprintf(tmp, sizeof(tmp), " IP:%03d", networkIPAddress);
    tempLen = strlen(tmp);
    if (tempLen < remainingLen) {
        remainingLen -= tempLen;
        strncat(dest, tmp, destLen);
    }

    switch (link) {
        case connected:
            snprintf(tmp, sizeof(tmp), " Link:%c%s", tstMember, "connected   "); break;
        case pending:
            snprintf(tmp, sizeof(tmp), " Link:%c%s", tstMember, "pending     "); break;
        case rejected:
            snprintf(tmp, sizeof(tmp), " Link:%c%s", tstMember, "rejected    "); break;
        case notVerified:
            snprintf(tmp, sizeof(tmp), " Link:%c%s", tstMember, "not verified"); break;
        default:
            snprintf(tmp, sizeof(tmp), " Link:%c%s", tstMember, "disconnected"); break;
    }
    tempLen = strlen(tmp);
    if (tempLen < remainingLen) {
        remainingLen -= tempLen;
        strncat(dest, tmp, destLen);
    }

    snprintf(tmp, sizeof(tmp), " Alive:member:%c", getAliveMember()? 'y':'n');
    tempLen = strlen(tmp);
    if (tempLen < remainingLen) {
        remainingLen -= tempLen;
        strncat(dest, tmp, destLen);
    }

    snprintf(tmp, sizeof(tmp), ", error:%c", getAliveError()? 'y':'n');
    tempLen = strlen(tmp);
    if (tempLen < remainingLen) {
        remainingLen -= tempLen;
        strncat(dest, tmp, destLen);
    }

    snprintf(tmp, sizeof(tmp), ", timeout:%d", getAliveTimeout());
    tempLen = strlen(tmp);
    if (tempLen < remainingLen) {
        remainingLen -= tempLen;
        strncat(dest, tmp, destLen);
    }
// ToDo(AME-Team) remove obsolete code

	/*** 02.05.2024 DS: outgoingRoute is not used any more
    switch (outgoingRoute) {
        case plcConfirmed:  sprintf(tmp, " Hin-Route: %s", "plcConfirmed"); break;
        case verified:      sprintf(tmp, " Hin-Route: %s", "verified");		break;
        default:            sprintf(tmp, " Hin-Route: %s", "inactive");
    }
    tempLen = strlen(tmp);
    if (tempLen < remainingLen) {
        remainingLen -= tempLen;
        strcat(dest, tmp);
    }
    ***/

    return dest;
}

NetworkMember::NetworkMember(): networkIPAddress(0Xff),
                                aliveTimeout(0), pendingTime(0),
                                link(disconnected),
                                outgoingRoute(unknown), testMember(false),
                                aliveMember(false), aliveError(false), fullMacAdrAvailable(false) {
    uint8_t tmp[] = {0xFF, 0x01, 0x01, 0xFF, 0xFE, 0x00, 0xFF, 0xFF };
    memcpy(macAddress, tmp, 8);
}

NetworkMember::~NetworkMember() {
}

void NetworkMember::setNetworkPanAddress(__attribute__((unused)) uint16_t panAddress) {
    // networkPANAddress = panAddress;
    // uint8Operations::insertUint16intoUint8Array(macAddress+1, panAddress);
}


void NetworkMember::resetMacAddress() {
    for (unsigned int i = 0; i < sizeof(macAddress); i++) {
      macAddress[i] = 0;
    }
}

void NetworkMember::adaptMacWithNetAddress() {
    uint8Operations::insertUint16intoUint8Array(macAddress+6, networkIPAddress);
}

void NetworkMember::setMacShortAddress(uint16_t macAdr) {
    uint8Operations::insertUint16intoUint8Array(macAddress+6, macAdr);
}

uint16_t NetworkMember::getMacShortAddress() {
    return uint8Operations::sum2BytesFromLSB(macAddress+6);
}

} /* namespace AME_SRC */


