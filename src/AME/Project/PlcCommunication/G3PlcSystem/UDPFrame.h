/*
 * UDPFrame.h
 *
 *  Created on: 16.04.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_UDPFRAME_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_UDPFRAME_H_

#include <cstdint>

namespace AME_SRC {

class UDPFrame {
 public:
    static const uint8_t ipv6HeaderLength = 40;
    struct IPv6Header {
        uint32_t Version :4;
        uint32_t TrafficClass :8;
        uint32_t FlowLabel :20;
        uint16_t PayloadLength;
        uint8_t NextHeader;
        uint8_t HopLimit;
        uint8_t *SourceAddress;
        uint8_t *DestinationAddress;
    };
    struct UDPformat {
        uint16_t SourcePort;
        uint16_t DestinationPort;
        uint16_t udpPackageLength;
        uint16_t UDPChecksum;
    };
    UDPFrame(IPv6Header newIPv6Header, UDPformat newUDPHeader, uint8_t *newPayload);
    virtual ~UDPFrame();
    uint8_t* toArray(uint8_t *nsduFrame);
    void calculateUDPChecksum();

 private:
    // static uint8_t udpFrameOutContainer[128] ;
    uint8_t* appendU16toU8Ptr(uint8_t *destPtr, uint16_t value);
    IPv6Header ipv6Header;
    UDPformat udpHeader;
    uint8_t *payload;
    uint32_t calculatePseudoHeadSum();
    uint32_t calculateUdpPackageSum();
    uint32_t calculatePayloadSum();
    uint16_t foldUint32ToUint16(uint32_t value);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_UDPFRAME_H_
