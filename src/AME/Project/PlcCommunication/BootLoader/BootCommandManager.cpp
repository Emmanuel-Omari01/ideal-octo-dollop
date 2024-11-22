/*
 * ModemBootCommand.cpp
 *
 *  Created on: 01.02.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "BootCommandManager.h"

#include "../../../../Renesas/NotGenerated/CPX_SRC.h"
#include "../../HelpStructures/Uint8Operations.h"

namespace AME_SRC {

const uint8_t *BootCommandManager::cpxFirmenwarePtr = body1;

  BootCommandManager::BootCommandManager() = default;

BootCommandManager::~BootCommandManager() = default;

/***
 * Short: Return the pointer to an segment which is referenced to a number.
 * Description: calculate the length of a segment according to the segment number and
 * forms for this length a start offset a pointer to the passed segment number.
 * This pointer is based on the start pointer of the CPX firmware
 * @param segmentNumber. The number of the segment of the CPX firmware
 * @return Pointer to the start of an segment in the CPX firmware
 */
const uint8_t* BootCommandManager::getSegmentDataPtrWithNumber(uint8_t segmentNumber) {
    const uint8_t *resultPtr;
    uint8_t SegmentDataStartByte = 80;
    uint32_t previousSegmentLength = calculateSegmentLengthTillNumber(segmentNumber);
    uint32_t tableOffset = SegmentDataStartByte + previousSegmentLength;
    resultPtr = &cpxFirmenwarePtr[tableOffset];
    return resultPtr;
}

/***
 * Short: Calculates the length of the segments from the passed number
 * @param segmentNumber The number of the segment of the CPX firmware
 * @return Length of the segments from the number passed
 */
uint32_t BootCommandManager::calculateSegmentLengthTillNumber(uint8_t segmentNumber) {
    uint32_t previousSegmentLength = 0;
    for (int segmentIndex = 0; segmentIndex < segmentNumber; segmentIndex++) {
        previousSegmentLength += calculateSegmentSize(segmentIndex);
    }
    return previousSegmentLength;
}

/***
 * Short: Returns the pointer to the information block of the passed segment number
 * @param segmentNumber Segment number of the CPX firmware
 * @return pointer to the information lock of the passed segment number
 */
const uint8_t* BootCommandManager::getSegmentInfoPtr(uint8_t segmentNumber) {
    const uint8_t *resultPtr;
    uint8_t InfoSegmentStartByte = 16;
    uint16_t tableOffset = InfoSegmentStartByte
            + segmentNumber * InfoSegmentByteLength;
    resultPtr = &cpxFirmenwarePtr[tableOffset];
    return resultPtr;
}

/***
 * Short: Calculates the length of the segment for the passed number
 * Description: This method calculates the length of the segment from the information block at the start of the CPX firmware
 * For this purpose, 4 bytes are added together at the correct position within the info block
 * @param segmentNumber segment number of the CPX firmware
 * @return length of the segment
 */
uint32_t BootCommandManager::calculateSegmentSize(uint8_t segmentNumber) {
    const uint8_t *segmentSizePtr;
    const uint8_t segmentSizeBytePosition = 8;
    uint32_t segmentLength = 0;
    segmentSizePtr = getSegmentInfoPtr(segmentNumber) + segmentSizeBytePosition;
    segmentLength = uint8Operations::sum4BytesFromMSB(segmentSizePtr);
    return segmentLength;
}

}  // namespace AME_SRC
