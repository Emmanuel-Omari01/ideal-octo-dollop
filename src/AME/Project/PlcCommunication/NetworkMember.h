/*
 * NetworkMember.h
 *
 *  Created on: 05.05.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_NETWORKMEMBER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_NETWORKMEMBER_H_

#include <cstdint>

namespace AME_SRC {

class NetworkMember {
 public:
    typedef enum {
        disconnected,
        pending,            // Registration phase join (Name: subscribing?)
        rejected,           // rejected by registration
        connected,          // registered and connected
        notVerified,
        maxEntry = notVerified
    } linkStatus;
    typedef enum {unknown, plcConfirmed, verified} routingStatus;
    enum {timeNotPending = 0xFF};

 private:
    uint8_t macAddress[8];
    uint16_t networkIPAddress;
//  uint16_t networkPANAddress;  // not needed
    uint16_t aliveTimeout;    // [s] is zeroed with each successful roundTrip
    uint8_t pendingTime;      // [60s]
    uint8_t link:3;           // {linkStatus}
    uint8_t outgoingRoute:2;  // {routingStatus}
    uint8_t testMember:1;
    uint8_t aliveMember:1;    // Peer is participant of the alive roundTrips
    uint8_t aliveError:1;     // an error occurred within last roundTrip
    uint8_t fullMacAdrAvailable:1;  // true: the MAC address is applied by an external device

 public:
    NetworkMember();
    NetworkMember(const uint8_t* macAddressPtr, uint16_t newNetworkIP);
    char* toString(char *dest, uint32_t destLen);
    bool hasEqualMacAddress(const uint8_t macAddress_[8]);
    virtual ~NetworkMember();
    void adaptMacWithNetAddress();
    uint16_t getNetworkIPAddress() const {
        return networkIPAddress;
    }

    void setNetworkIPAddress(uint16_t networkAddress) {
        this->networkIPAddress = networkAddress;
    }

    void setNetworkPanAddress(uint16_t panAddress);
    void resetMacAddress();
    void fillMacAddress(const uint8_t *macAddressPtr);

    const uint8_t* getMacAddress() {
        return macAddress;
    }

    uint16_t getMacShortAddress();
    void setMacShortAddress(uint16_t macAdr);


    void setLink(linkStatus status) {
        link = status & 0x7;
    }

    linkStatus getLink() const {
        return (linkStatus) link;
    }

    bool connectedOrNotVerified() {
        return (link == connected) || (link == notVerified);
    }

    uint8_t getOutgoingRoute() const {
        return outgoingRoute;
    }

    void setOutgoingRoute(uint8_t status) {
        this->outgoingRoute = status & 0x3;
    }

    uint8_t getPendingTime() const {
        return pendingTime;
    }

    void setPendingTime(uint8_t time) {
        this->pendingTime = time;
    }

    bool getTestMember() const {
        return testMember;
    }
    void setTestMember(bool set) {
        testMember = set;
    }

    bool getAliveMember() const {
       return aliveMember;
    }
    void setAliveMember(bool set) {
        aliveMember = set;
    }

    bool getAliveError() const {
       return aliveError;
    }
    void setAliveError(bool set) {
        aliveError = set;
    }

    uint16_t getAliveTimeout() const {
       return aliveTimeout;
    }
    void setAliveTimeout(uint16_t time_s) {
        aliveTimeout = time_s;
    }

    bool hasFullMacAddress() const {
        return fullMacAdrAvailable;
    }
    void setBitFullMacAddress(bool set) {
        fullMacAdrAvailable = set;
    }
};

} /* namespace AME_SRC */

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_NETWORKMEMBER_H_
