/*
 * UDPFrame.cpp
 *
 *  Created on: 16.04.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "UDPFrame.h"

#include <cstring>

namespace AME_SRC {

// uint8_t UDPFrame::udpFrameOutContainer[128] ;

UDPFrame::UDPFrame(IPv6Header newIPv6Header, UDPformat newUDPHeader,
        uint8_t *newPayload) :
        ipv6Header(newIPv6Header), udpHeader(newUDPHeader), payload(newPayload) {
}

UDPFrame::~UDPFrame() = default;

/***
 * Converts the data structures of the classes into an merged array.
 * @nsduFrame. Container for Header, payload, checksum. The container must be kept in the calling routine in sufficient size.
 * @return array which contains ipv6Header, UDPHeader and the data.
 */
uint8_t* UDPFrame::toArray(uint8_t* nsduFrame) {
    enum { udpHeaderLength = 8, firstPartLength = 4 };
    uint8_t *udpFramePtr, *tmp;
    udpFramePtr = nsduFrame;    // udpFrameOutContainer;
    tmp = udpFramePtr;
    udpFramePtr[0] = (uint8_t) ((ipv6Header.Version << 4)
            + (ipv6Header.TrafficClass & 0xF0));
    udpFramePtr[1] = (uint8_t) (((ipv6Header.TrafficClass & 0x0F) << 4)
            + (ipv6Header.FlowLabel & 0xF0000));
    udpFramePtr[2] = (uint8_t) (ipv6Header.FlowLabel & 0x0FF00);
    udpFramePtr[3] = (uint8_t) (ipv6Header.FlowLabel & 0x000FF);
    udpFramePtr = appendU16toU8Ptr(udpFramePtr + firstPartLength,
            ipv6Header.PayloadLength);
    *(udpFramePtr++) = ipv6Header.NextHeader;
    *(udpFramePtr++) = ipv6Header.HopLimit;
    memcpy(udpFramePtr, ipv6Header.SourceAddress, 16);
    udpFramePtr += 16;
    memcpy(udpFramePtr, ipv6Header.DestinationAddress, 16);
    udpFramePtr += 16;
    udpFramePtr = appendU16toU8Ptr(udpFramePtr, udpHeader.SourcePort);
    udpFramePtr = appendU16toU8Ptr(udpFramePtr, udpHeader.DestinationPort);
    udpFramePtr = appendU16toU8Ptr(udpFramePtr, udpHeader.udpPackageLength);
    udpFramePtr = appendU16toU8Ptr(udpFramePtr, udpHeader.UDPChecksum);
    memcpy(udpFramePtr, payload, udpHeader.udpPackageLength - udpHeaderLength);
    udpFramePtr[ipv6HeaderLength + ipv6Header.PayloadLength - 1] = 0;  // String termination
    udpFramePtr = tmp;
    // HDLC Escape Sequence take into account here? (0x7E and 0x7D is converted to 0x7D 0x5E)
    // The function check_HDLCsequence is called here: PlcCommand::toRawString2()
    return tmp;
}

/***
 * Calculate the checksum of the UDP header.
 */
void UDPFrame::calculateUDPChecksum() {
    uint32_t pseudoHeadSum = calculatePseudoHeadSum();
    uint32_t udpSum = calculateUdpPackageSum();
    uint16_t checkSum = foldUint32ToUint16(pseudoHeadSum + udpSum);
    udpHeader.UDPChecksum = ~checkSum;
}

/***
 * Calculate the pseudo sum of the UDP header which is need for the calculation of the checksum.
 * @return pseudo sum of the UDP header
 */
uint32_t UDPFrame::calculatePseudoHeadSum() {
    enum { ipV6adress8BitLength = 16, udpProtocollType = 0x11 };
    uint32_t pseudoHeadSum = 0;
    uint32_t sourceAddressSum = 0;
    uint32_t destinationAddressSum = 0;

    for (int i = 0; i < ipV6adress8BitLength; i += 2) {
      sourceAddressSum += ((ipv6Header.SourceAddress[i] << 8) + ipv6Header.SourceAddress[i + 1]);
      destinationAddressSum += ((ipv6Header.DestinationAddress[i] << 8) + ipv6Header.DestinationAddress[i + 1]);
    }
    pseudoHeadSum = sourceAddressSum + destinationAddressSum + udpProtocollType + udpHeader.udpPackageLength;
    return pseudoHeadSum;
}

/***
 * Calculates the UDP packet sum for the checksum calculation.
 * @return Packet sum
 */
uint32_t UDPFrame::calculateUdpPackageSum() {
    uint32_t payloadSum = calculatePayloadSum();
    uint32_t udpHeaderSum = udpHeader.DestinationPort + udpHeader.SourcePort + udpHeader.udpPackageLength;
    return (udpHeaderSum + payloadSum);
}

/***
 * Calculates the sum of the user data (payload)
 * @return sum
 */
uint32_t UDPFrame::calculatePayloadSum() {
    uint32_t payloadSum = 0;
    const uint8_t udpHeaderLength = 8;
    const uint16_t payloadLength = udpHeader.udpPackageLength - udpHeaderLength;
    for (int i = 0; i < payloadLength - 1; i += 2) {
        payloadSum += (payload[i] << 8) + payload[i + 1];
    }
    if ((payloadLength % 2) == 1) {
        payloadSum += payload[payloadLength - 1] << 8;
    }
    return payloadSum;
}

/***
 * Fold an Uint32 to an Uint16
 * @param value Uint32
 * @return Uint16
 */
uint16_t UDPFrame::foldUint32ToUint16(uint32_t value) {
    const uint32_t mask = 0xFFFF;
    while ((value >> 16u) != 0UL) {
        value = (value & mask) + (value >> 16u);
    }
    return value;
}


/***
 * Add an uint16 value to an array of uint8
 * @param destPtr target array
 * @param value uint16
 * @return target array
 */
uint8_t * UDPFrame::appendU16toU8Ptr(uint8_t *destPtr, uint16_t value) {
    *(destPtr++) = (value >> 8);
    *(destPtr++) = value & 0xff;
    return destPtr;
}

}  // namespace AME_SRC

