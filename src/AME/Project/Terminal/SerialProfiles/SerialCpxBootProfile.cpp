/*
 * SerialCpxBootProfile.cpp
 *
 *  Created on: 20.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "SerialCpxBootProfile.h"

#include <cstring>

namespace AME_SRC {

SerialCpxBootProfile::SerialCpxBootProfile(ModemInterpreter *modemPtr) :
        modemPtr_ { modemPtr } {
    cmdLines.indexIN = 0;
    cmdLines.indexOUT = 0;
}

SerialCpxBootProfile::~SerialCpxBootProfile() = default;

void SerialCpxBootProfile::interpretReceivedLetter(char letter) {
    if (modemPtr_) {
        switch (letter) {
        case BootCommandManager::SegmentRequest0:
            cmdLines.container[cmdLines.indexIN++][0] = letter;
            // SerialComInterface::getInstance(sci5)->pushTXD(outPutLetter);
#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
            modemPtr_->setIsNotUploading(false);
#endif
            modemPtr_->handelBootloading();
            return;
        case BootCommandManager::SegmentRequest3:
        case BootCommandManager::SegmentRequest1:
        case BootCommandManager::SegmentRequest2:
        case BootCommandManager::BaudrateChangeRequest:
        case BootCommandManager::BaudrateSetAccept:
        case BootCommandManager::FirmenwarerTansmissionSucess:
#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
            modemPtr_->setIsNotUploading(true);
#endif
            cmdLines.container[cmdLines.indexIN][0] = letter;
            if (++cmdLines.indexIN >= cmdCount) {
                cmdLines.indexIN = 0;
            }
            return;
        default:
            break;
        }
    }
}

char* SerialCpxBootProfile::getBuffer() {
    char *p;
    if (cmdLines.indexOUT != cmdLines.indexIN) {
        p = cmdLines.container[cmdLines.indexOUT];
        // Correct ring buffer pointer
        if (++cmdLines.indexOUT >= cmdCount) {
            cmdLines.indexOUT = 0;
        }
        return p;
    } else {
        return 0;
    }
}

void SerialCpxBootProfile::setBuffer(uint8_t count, char *array) {
}

}  // namespace AME_SRC
