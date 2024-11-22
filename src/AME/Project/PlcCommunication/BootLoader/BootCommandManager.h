/*
 * ModemBootCommand.h
 *
 *  Created on: 01.02.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_BOOTLOADER_BOOTCOMMANDMANAGER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_BOOTLOADER_BOOTCOMMANDMANAGER_H_

#include <cstdint>

namespace AME_SRC {

class BootCommandManager {
 private:
    static const char baudrateChangeComplet = 0xAA;
    static const char baudrateSetAccept = 0xCF;
    static const char baudrateSetRequest = 0xC1;
    static const uint8_t *cpxFirmenwarePtr;
    static const uint8_t InfoSegmentByteLength = 16;
    static uint32_t calculateSegmentLengthTillNumber(uint8_t segmentNumber);

 public:
    static const uint8_t OffsetAdressByteLength = 4;
    typedef enum {
        bps9600 = 0x0,
        bps19200 = 0x1,
        bps38400 = 0x2,
        bps57600 = 0x3,
        bps115200 = 0x4,
        bps230400 = 0x5,
        bps300k = 0x6,
        bps375k = 0x7,
        bps460_8k = 0x8,
        bps500k = 0x9,
        bps750k = 0xA,
        bps937_5k = 0xB,
        bps1000k = 0xC,
    } Baudrate;
    typedef enum {
        SegmentRequest0 = 0x80,
        SegmentRequest1 = 0x81,
        SegmentRequest2 = 0x82,
        SegmentRequest3 = 0x83,
        BaudrateChangeRequest = 0xA1,
        BaudrateUsedSucess = 0xA,
        FirmenwarerTansmissionSucess = 0xB0,
        BaudrateSet = 0xC,
        BaudrateSetAccept = 0xCF,
        OPERATE = 0x1,
        ERROR = 0x0,
    } CommandID;

    BootCommandManager();

    static const uint8_t* getSegmentDataPtrWithNumber(uint8_t segmentNumber);
    static const uint8_t* getSegmentInfoPtr(uint8_t segmentNumber);
    virtual ~BootCommandManager();

    static char getBaudrateChangeComplet() {
        return baudrateChangeComplet;
    }

    static bool isBaudrateSetAccepted(char answer) {
        return answer == baudrateSetAccept;
    }

    static char getBaudrateSetRequest() {
        return baudrateSetRequest;
    }

    static uint32_t calculateSegmentSize(uint8_t segmentNumber);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_BOOTLOADER_BOOTCOMMANDMANAGER_H_
