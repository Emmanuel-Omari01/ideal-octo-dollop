/*
 * ModemBootLoader.cpp
 *
 *  Created on: 01.02.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "ModemBootLoader.h"

#include <cstring>

#include "../../../../Renesas/NotGenerated/crc32.h"
#include "../../SystemBase/Global2.h"

namespace AME_SRC {

bool ModemBootLoader::firmenwareStatusUnknown = false;
bool ModemBootLoader::busyState = false;
bool ModemBootLoader::timerStopped = false;
ModemBootLoader::bootStates ModemBootLoader::transmittState = firstRequest;
const uint8_t *ModemBootLoader::nextTransmissionBlockPtr = NULL;
CompareMatchTimeDriver *ModemBootLoader::cmtPtr = Global2::getCmt1();
SerialDriver *ModemBootLoader::modemInterface = 0;
uint32_t ModemBootLoader::headerMessageLength = 12;
uint32_t ModemBootLoader::fragmentMessageLength = 1024;
int ModemBootLoader::blockFragmentCount = -1;
// const uint8_t* ModemBootLoader::cpxFirmenwarePtr=
ModemBootLoader::ModemBootLoader(SerialDriver *driver,
                                 BootCommandManager::Baudrate newFlashBaud,
                                 BootCommandManager::Baudrate newPlcBaud) :
        isNewSegmentBlock(false), sgementNumber(-1), flashBaud(newFlashBaud), plcBaud(
                newPlcBaud) {
    cmtPtr->setMatchInterruptFuncPtr(interruptPrintNextTransmission);
    firmenwareStatusUnknown = true;
    modemInterface = driver;
    SerialComInterface::sci5TransmissionEndInterruptPtr =
            startCompareMatchTimer;
}

ModemBootLoader::~ModemBootLoader() = default;

/***
 * Name: reactOnRequest
 *
 * Description: This Method reacts on requests relating the Transmission of CPX-Firmware. The first Part is calculating the Firmware Fragments of
 * one Firmware Segment. This Behavior is started by Passing "OPERATE" as BootCommand to the Method. The second part handles the Requests of the
 * CPX-Modem which concerning "changing the baudrate" or "asking for new Segments"
 *
 * Reference: "Boot Operating Manual"
 *
 * @param command	Message from the CPX-Modem or Operate Command from the System
 */
void ModemBootLoader::reactOnRequest(BootCommandManager::CommandID command) {
    uint8_t *result = 0;
//  if(command == BootCommandManager::OPERATE) {
//      getFirmenwareFragment(modemInterface);
//  }
//  else {
//  }
    result = answerRequest(command);
    if (result) {
        modemInterface->printRawFast((char*) result, getBootMessageLength());
    }
}

/***
 * Name: getFirmenwareFragment
 *
 * Description: This Method build the current Fragment of a CPX-Firmware-Segment which can be transmitted to the Modem. The Transmission of one
 * Segment is divided into several Fragments(1024 Byte) which are surrounded by breaks(around 560µs). In front of each Segment is an information
 * command.
 *
 * Reference: "Boot Operating Manual"
 *
 * @return current Fragment or Command of the  segment transmission
 */
/***
 uint8_t* ModemBootLoader::getFirmenwareFragment(SerialDriver* driver) {
 uint8_t* result = 0;
 if(driver->isTransmissionComplete()){
 const int messageOffset = 1024;

 const uint16_t blockSize = 16*64;
 static uint32_t segmentLength = 0;
 uint16_t endLength = 0;

 if(isNewSegmentBlock){
 transmittState = firstBreak;
 blockFragmentCount = -1;
 }

 switch(transmittState){
 case firstBreak:
 segmentLength = BootCommandManager::calculateSegmentSize(sgementNumber);
 transmittState = startBreakTimer;
 headerMessageLength = messageOffset;
 isNewSegmentBlock = false;
 break;
 case segmentTransmissionStart:
 nextTransmissionBlockPtr = (uint8_t*)BootCommandManager::getSegmentDataPtrWithNumber(sgementNumber);
 nextTransmissionBlockPtr = (nextTransmissionBlockPtr +blockFragmentCount*blockSize);
 transmittState = startBreakTimer;
 break;
 case startBreakTimer:
 cmtPtr->start();
 transmittState = waitBreakTimer;
 break;
 case waitBreakTimer:
 break;
 case segmentCondition:
 if (++blockFragmentCount < (segmentLength/blockSize)) {
 transmittState = segmentTransmissionStart;
 } else {
 transmittState = segmentEnd;
 }
 if(blockFragmentCount == 3){
 asm ("nop");
 }
 result = NULL;
 break;
 case segmentEnd:
 endLength = segmentLength - blockFragmentCount*blockSize;
 nextTransmissionBlockPtr = (uint8_t*)BootCommandManager::getSegmentDataPtrWithNumber(sgementNumber)+blockFragmentCount*blockSize;
 headerMessageLength = endLength;
 transmittState = startBreakTimer;
 blockFragmentCount = -2;
 break;
 }
 }
 return result;

 }
 ***/

void ModemBootLoader::updateTransmissionBlockPtr() {
    const uint16_t blockSize = 16 * 64;
    uint16_t endLength = 0;
    static uint32_t segmentLength = 0;
    static enum {
        start, setBlock, waitTimer, proofEnd, setEnd, waitEnd, idle
    } transmissionStates = idle;

    if (isNewSegmentBlock) {
        transmissionStates = start;
        blockFragmentCount = 0;
    }

    switch (transmissionStates) {
    case start:
        segmentLength = BootCommandManager::calculateSegmentSize(sgementNumber);
        isNewSegmentBlock = false;
        transmissionStates = setBlock;
        fragmentMessageLength = 1024;
        break;
    case setBlock:
        nextTransmissionBlockPtr =
                BootCommandManager::getSegmentDataPtrWithNumber(sgementNumber);
        nextTransmissionBlockPtr = (nextTransmissionBlockPtr
                + blockFragmentCount * blockSize);
        transmissionStates = waitTimer;
        break;
    case waitTimer:
        if (timerStopped) {
            transmissionStates = proofEnd;
            timerStopped = false;
        }
        break;
    case proofEnd:
        if (++blockFragmentCount < (segmentLength / blockSize)) {
            transmissionStates = setBlock;
        } else {
            transmissionStates = setEnd;
        }
        break;
    case setEnd:
        endLength = segmentLength - blockFragmentCount * blockSize;
        nextTransmissionBlockPtr =
                (uint8_t*) BootCommandManager::getSegmentDataPtrWithNumber(
                        sgementNumber) + blockFragmentCount * blockSize;
        fragmentMessageLength = endLength;
        transmissionStates = waitEnd;
        break;
    case waitEnd:
        if (timerStopped) {
            transmissionStates = idle;
            blockFragmentCount = -2;
            timerStopped = false;
        }
        break;
    }
}

void ModemBootLoader::interruptPrintNextTransmission() {
    if (nextTransmissionBlockPtr) {
        SerialComInterface::setFastMode(true);
        modemInterface->printRawFast((char *) nextTransmissionBlockPtr, fragmentMessageLength);
        cmtPtr->stop();
        timerStopped = true;
    }
}

/***
 * Name: answerRequest
 *
 * Description: This Method creates an answer string which handles the Baudrate change Request or 4 Segment Requests.
 * Concerning the baudrate change two baudrates are transmitted, one for Flashing the remaining CPX Firmware and another
 * one for PLC Communication. If a Segment Request is detected the Host answers with an information command for the
 * specific Command.
 *
 * Reference: "Boot Operating Manual"
 *
 * @param request	CPX Modem Request (1 Byte)
 *
 * @return	Host Answer
 */
uint8_t* ModemBootLoader::answerRequest(BootCommandManager::CommandID request) {
    uint8_t *answer = NULL;
    switch (request) {
    case BootCommandManager::SegmentRequest0:  // NO BREAK
    case BootCommandManager::SegmentRequest3:  // NO BREAK
    case BootCommandManager::SegmentRequest2:  // NO BREAK
    case BootCommandManager::SegmentRequest1: {
        const uint8_t* tmp;
        busyState = true;
        headerMessageLength = 12;
        sgementNumber = request - BootCommandManager::SegmentRequest0;
        tmp = BootCommandManager::getSegmentInfoPtr(sgementNumber);
        tmp+= BootCommandManager::OffsetAdressByteLength;
        answer = (uint8_t *) tmp;
        isNewSegmentBlock = true;
    }
        break;
    case BootCommandManager::BaudrateChangeRequest: {
        char baudrateSetLetter = BootCommandManager::getBaudrateSetRequest();
        modemInterface->printRawFast(&baudrateSetLetter, 1);
        wait560MikrosecAfterTransmission(modemInterface, false);
        char setInformation = static_cast<char>((BootCommandManager::bps750k << 4)
                | BootCommandManager::bps750k);  // Flash-Baudrate | PLC-Baudrate
        modemInterface->printRawFast(&setInformation, 1);
        headerMessageLength = 0;
        answer = NULL;
        busyState = false;
    }
        break;
    case BootCommandManager::BaudrateSetAccept: {
        modemInterface->getSciPtr()->changeBaudRate(baud750000);
        char changeCompleteLetter =
                BootCommandManager::getBaudrateChangeComplet();
        modemInterface->printRawFast(&changeCompleteLetter, 1);
        headerMessageLength = 0;
        answer = NULL;
        *answer = BootCommandManager::getBaudrateChangeComplet();
    }
        break;
    case BootCommandManager::FirmenwarerTansmissionSucess: {
        modemInterface->getSciPtr()->setFastMode(false);
        firmenwareStatusUnknown = false;
        busyState = false;
        headerMessageLength = 0;
        sgementNumber = -1;
        answer = NULL;
    }
        break;
    default:
        break;
    }
    return answer;
}

/***
 bool ModemBootLoader::isOldSegmentNumber(int currentSegNumber){
 static int previousSegNumber = -1;
 if( previousSegNumber ==  currentSegNumber){
 previousSegNumber = -1;
 return true;
 }
 else{
 previousSegNumber = currentSegNumber;
 return false;
 }

 }
 ***/

/***
 * Name: wait560MikrosecAfterTransmission
 *
 * Description: The System waits after Transmission 560µs
 *
 * @param driver Serial Driver for the Transmission
 */
void ModemBootLoader::wait560MikrosecAfterTransmission(SerialDriver *driver,
                                                       bool waitTransmission) {
    if (waitTransmission == true) {
        while (!driver->isTransmissionComplete()) {
            asm("nop");
        }
    }
    for (int i = 0; i < 1423; i++) {
        asm("nop");
    }
}

bool ModemBootLoader::isFirmenwareInStorage() {
    static bool storageIsTested = false;
    static uint32_t crcExpected = 0x17594FD5;
    if (storageIsTested) {
        return true;
    } else {
        const uint8_t *startPointer = BootCommandManager::getSegmentInfoPtr(0);
        storageIsTested = CRC32::crc32_check(startPointer,(uint16_t) 64, crcExpected);
        return storageIsTested;
    }
}

}  // namespace AME_SRC

