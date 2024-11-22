/*
 * PlcCommand.cpp
 *
 *  Created on: 16.02.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "PlcCommand.h"

#include <cstring>
#include <cstdio>

#include "../../../../Renesas/NotGenerated/crc32.h"
#include "../../SystemBase/Global2.h"
#include "../../Terminal/SerialProfiles/SerialProfile.h"
#include "../../HelpStructures/Uint8Operations.h"
#include "UDPFrame.h"

namespace AME_SRC {
// using namespace  PLC_CommandTypeSpecification;

uint8_t PlcCommand::outputBuffer[kBufSize] = {0};
uint8_t PlcCommand::nextCmdNr = 0;

PlcCommand::PlcCommand(PLCBlock blockType, CHANNEL channel, ACESSTyp accessTyp,
                       LAYER accessLayer, uint8_t command, uint8_t *param,
                       uint8_t paramLength) {
    header.reserve = 0;
    header.plcBlockType = blockType;

    payload.channelID = channel & 0x01;
    payload.acessTyp = accessTyp & 0x07;
    payload.targetLayer = accessLayer & 0x0F;
    payload.command = command;
    copyNBytesFromArrayToParameter(paramLength, param);
    crc = 0;
}

PlcCommand::~PlcCommand() = default;

/***
 * This method converts the PLC command into a readable character string
 * @return readable character string
 */

char *PlcCommand::toCommandStringHead() const {
    static char commandString[10 + 10 + 12 + 15 + 13 + 14 + 1];
    char *cPartPtr = 0;
    snprintf(commandString, sizeof(commandString), "|%s|",
            (header.plcBlockType == 0) ? "SysBlock" : "G3-Block");
    snprintf(commandString + 10, sizeof(commandString)-10, "|Channel %d|", getChannel());

    switch (getAccesType()) {
    case request:
        cPartPtr = const_cast<char*>("|request---|");
        break;
    case confirm:
        cPartPtr = const_cast<char*>("|confirm---|");
        break;
    case indication:
        cPartPtr = const_cast<char*>("|indication|");
        break;
    default:
        cPartPtr = const_cast<char*>("|error-----|");
        break;
    }
    snprintf(commandString + 20, sizeof(commandString)-20, "%s", cPartPtr);
    switch (getAccesLayer()) {
    case g3Controller:
        cPartPtr = const_cast<char*>("|G3 Controller|");
        break;
    case umacLayer:
        cPartPtr = const_cast<char*>("|UMAC Layer---|");
        break;
    case adpLayer:
        cPartPtr = const_cast<char*>("|ADP Layer----|");
        break;
    case eapLayer:
        cPartPtr = const_cast<char*>("|EAP Layer----|");
        break;
    default:
        cPartPtr = const_cast<char*>("|error--------|");
        break;
    }
    snprintf(commandString + 32, sizeof(commandString)-32, "%s", cPartPtr);
    snprintf(commandString + 47, sizeof(commandString)-47, "|Command: %2d|", getCommandID());
    snprintf(commandString + 60, sizeof(commandString)-60,  "|Status: 0x%02X|", getParameter()[0]);
    return commandString;
}

char* PlcCommand::toCommandStringData(uint8_t partNr, bool *done) const {
    char *rawString = this->toRawString();
    uint32_t n, ofs;
    if (this->payload.targetLayer == adpLayer
            && this->payload.acessTyp == request
            && this->payload.command == 0) {
        if (partNr == 0) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "nsduLength: 0x");
            for (int i = 0; i < 2; i++) {
                ofs = 14 + i * 2;
                snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                        "%02X", rawString[i + 5]);
            }
            snprintf(Global2::OutBuff + 18, Global2::outBuffMaxLength -18, " IPV6Metadata: 0x");
            for (int i = 0; i < 4; i++) {
                ofs = 18 + 17 + i * 2;
                snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                        "%02X", rawString[i + 7]);
            }
            ofs = 18 + 17 + 8;
            snprintf(Global2::OutBuff + ofs, Global2::outBuffMaxLength -ofs, " PayloadLength: 0x");
            for (int i = 0; i < 2; i++) {
                ofs = 18 + 17 + 8 + 18 + i * 2;
                snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                        "%02X", rawString[i + 11]);
            }
            ofs = 18 + 17 + 8 + 18 + 4;
            snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                    "\nNextHeader: %02X HopLimit: %02X", rawString[13], rawString[14]);
            ofs = 18 + 17 + 8 + 18 + 4 + 28;
            snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                    "\nSourceAddress: ");
            for (int i = 0; i < 16; i++) {
                ofs = 18 + 17 + 8 + 18 + 4 + 28 + 16 + i*2;
                snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                     "%02X", rawString[i + 15]);
            }
            ofs = 18 + 17 + 8 + 18 + 4 + 28 + 16 + 32;
            snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                    "\nDestAddress: ");
            for (int i = 0; i < 16; i++) {
                ofs = 18 + 17 + 8 + 18 + 4 + 28 + 16 + 32 + 14 + i * 2;
                snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                      "%02X", rawString[i + 31]);
            }
            ofs = 18 + 17 + 8 + 18 + 4 + 28 + 16 + 32 + 14 +32;
            snprintf(const_cast<char*>(&Global2::OutBuff[ofs]), Global2::outBuffMaxLength -ofs,
                     "\nUDPFrame: ");
            *done = false;
        } else {
            n = 0;   // +18+17+8+18+4+28+16+32+14+32+11+
            for (int i = 0; i < payload.parameterLength - 45; i++) {
                if (n < sizeof(Global2::OutBuff)) {
                    snprintf(const_cast<char*>(&Global2::OutBuff[n]), Global2::outBuffMaxLength -n,
                            "%02X", rawString[i + 47]);
                } else {
                    break;
                }
                n += 2;
            }
            *done = true;
        }
    } else {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "Data: ");
        n = 6;
        for (int i = 0; i < payload.parameterLength; i++) {
            if (n < sizeof(Global2::OutBuff)) {
                snprintf(const_cast<char*>(&Global2::OutBuff[n]), Global2::outBuffMaxLength -n,
                        "%02X", rawString[i + 5]);
            } else {
                break;
            }
            n += 2;
        }
        *done = true;
    }
    // char* tmp = new char[6+stringlength*2+1];

// sprintf("nsduLength: %d",requestString,uint8Operations::sum2BytesFromLSB(requestPtr->getParameter()));
// ModemInterpreter::modemCrierPtr->println(requestString);
// strncpy(requestString,dataContainer,4);
// sprintf("IPv6 Metadata: %s",Global2::OutBuff,requestString);
// ModemInterpreter::modemCrierPtr->println(Global2::OutBuff);
// dataContainer+=4;
// strncpy(requestString,dataContainer,2);
// sprintf("PayloadLength: %s",Global2::OutBuff,requestString);
// ModemInterpreter::modemCrierPtr->println(Global2::OutBuff);
// dataContainer+=2;
// strncpy(requestString,dataContainer,2);
// sprintf("NextHeader: %c HopLmit: %c",Global2::OutBuff,requestString[0],requestString[1]);
// ModemInterpreter::modemCrierPtr->println(Global2::OutBuff);
// dataContainer+=2;
// strncpy(requestString,dataContainer,16);
// sprintf("SourceAdress: %s",Global2::OutBuff,requestString);
// ModemInterpreter::modemCrierPtr->println(Global2::OutBuff);
// dataContainer+=16;
// strncpy(requestString,dataContainer,16);
// sprintf("DestinationAdress: %s",Global2::OutBuff,requestString);
// ModemInterpreter::modemCrierPtr->println(Global2::OutBuff);
// dataContainer+=16;
    return Global2::OutBuff;
}

/***
 * This method calculates the checksum of the PLC command with stored parameters.
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void PlcCommand::calculateCRC() {
    const uint8_t headerLength = 2;
    const uint8_t payloadSpecificationLength = 2;
    uint8_t frame[kBufSize];
    uint16_t length = (uint16_t) (headerLength + payloadSpecificationLength
            + payload.parameterLength);
    memmove(&frame[0], &header.reserve, headerLength);
    frame[2] = (uint8_t) ((payload.channelID << 7) | (payload.acessTyp << 4)
            | (payload.targetLayer));
    frame[3] = payload.command;
    memmove(&frame[4], payload.parameter, payload.parameterLength);
    crc = CRC32::crc32_calc(frame, length);
}

CHANNEL PlcCommand::getChannel() const {
    return (CHANNEL) payload.channelID;
}

ACESSTyp PlcCommand::getAccesType() const {
    return (ACESSTyp) payload.acessTyp;
}

LAYER PlcCommand::getAccesLayer() const {
    return (LAYER) payload.targetLayer;
}

uint8_t PlcCommand::getCommandID() const {
    return payload.command;
}

const uint8_t* PlcCommand::getParameter() const {
    return payload.parameter;
}


uint16_t PlcCommand::getSrcIPFromDataCMD(const PlcCommand &cmd) {
    enum { srcIpOffset = 24 };
    return uint8Operations::sum2BytesFromLSB(reinterpret_cast<const uint8_t*>(cmd.payload.parameter + srcIpOffset));
}

PlcCommand::PlcCommand() :
        crc(0), confirmIndex(0) {
    header.reserve = 0;
    header.plcBlockType = 0;
    payload.parameterLength = 0;
    payload.command = 0;
    messagePtr = 0;
    incCmdNr();
}

PlcCommand::PlcCommand(char *buffer) {
    enum bytePositionWithoutFraming {
        reservePosition = 0,
        blockTypePosition = 1,
        firstPayloadPosition = 2,
        commandPosition = 3,
        dataPPosition = 4,
        crcLength = 4
    };
    char *pStart = buffer;
    // char *pEnde  = buffer;
    uint16_t parameterByteCounter = 0;
    int16_t i;  // ,j;
    buffer = trimCommand(buffer);
    header.reserve = buffer[reservePosition];
    header.plcBlockType = buffer[blockTypePosition];
    convertByteToFirstPaylodPart(buffer[firstPayloadPosition]);
    payload.command = buffer[commandPosition];
    buffer += dataPPosition;
    while (buffer[parameterByteCounter] != framingCode) {
        parameterByteCounter++;
    }
    // look for the last occurrence of framingCode
    // Requirement: the buffer has previously been filled with '0', for example
    i = SerialProfile::cmdLength - 1;
    while (i > 0) {
        char test = pStart[i];
        if (test == framingCode) {
            // pEnde = &pStart[i];
            break;
        }
        i--;
    }
    // parameterByteCounter = pEnde - buffer -1;
    payload.parameterLength = (uint8_t) (parameterByteCounter + 1 - crcLength);
    rollBack_HDLCSequence(pStart, &i);

    // payload.parameter= new uint8_t[payload.parameterLength];
    copyNBytesFromArrayToParameter(payload.parameterLength, reinterpret_cast<uint8_t*>(buffer));
    // memmove(payload.parameter,(uint8_t *)buffer,payload.parameterLength);
    memmove(&crc, buffer + payload.parameterLength, crcLength);
}

PlcCommand::PlcCommand(const PlcCommand &cmd) {
    header.reserve = cmd.header.reserve;
    header.plcBlockType = cmd.header.plcBlockType;

    payload.channelID = cmd.payload.channelID;
    payload.acessTyp = cmd.payload.acessTyp;
    payload.targetLayer = cmd.payload.targetLayer;
    payload.command = cmd.payload.command;
    payload.parameterLength = cmd.payload.parameterLength;
    this->copyNBytesFromArrayToParameter(cmd.payload.parameterLength,
            (uint8_t *) cmd.payload.parameter);
    crc = cmd.crc;
    confirmIndex = cmd.confirmIndex;
    messagePtr = cmd.messagePtr;
}

char* PlcCommand::trimCommand(char *buffer) {
    int plcFrame = 0x7E;
    int index = 0;
    while (buffer[index] != plcFrame) {
        index++;
    }
    return buffer + index + 1;
}

/***
 * Conversion into a raw string.
 * This method converts the PLC command into a character string in which
 * the characters are stored including the header and payload structure
 * @return string with values of the header and payload structure.
 */
char* PlcCommand::toRawString() const {
    uint16_t length = getStringLength();
    outputBuffer[0] = framingCode;
    memmove(&outputBuffer[1], &header.reserve, 2);
    uint8_t debug = (uint8_t) ((payload.channelID << 7)
            | (payload.acessTyp << 4) | (payload.targetLayer));
    outputBuffer[3] = debug;
    outputBuffer[4] = payload.command;
    memmove(&outputBuffer[5], payload.parameter, payload.parameterLength);
    // calculateCRC();
    writeUInt32ToBufferBigEndian(crc, (uint8_t*) &outputBuffer[length - 5]);
    // check_HDLCsequence();
    return reinterpret_cast<char *>(outputBuffer);
}

char* PlcCommand::toRawString2() {
    uint16_t length = getStringLength();
    outputBuffer[0] = framingCode;
    memmove(&outputBuffer[1], &header.reserve, 2);
    uint8_t debug = (uint8_t) ((payload.channelID << 7)
            | (payload.acessTyp << 4) | (payload.targetLayer));
    outputBuffer[3] = debug;
    outputBuffer[4] = payload.command;
    memmove(&outputBuffer[5], payload.parameter, payload.parameterLength);
    if (crc == 0) {
        calculateCRC();
    }
    writeUInt32ToBufferBigEndian(crc, (uint8_t*) &outputBuffer[length - 5]);
    check_HDLCsequence();
    return reinterpret_cast<char *>(outputBuffer);
}

/***
 * Converts one byte to ChannelID, access type and target layer.
 * @param firstPayloadPart byte witch has to be converted
 */
void PlcCommand::convertByteToFirstPaylodPart(uint8_t firstPayloadPart) {
    payload.channelID = (firstPayloadPart & 0x80) != 0;
    payload.acessTyp = (firstPayloadPart & 0x70) >> 4;
    payload.targetLayer = firstPayloadPart & 0x0F;
}

/***
 * This method converts a uint32 type into an array of 4 fields of type uint8
 * @param number uint32 type
 * @param buffer target array
 */
void PlcCommand::writeUInt32ToBufferBigEndian(uint32_t number, uint8_t *buffer) const {
    buffer[0] = (uint8_t) ((number >> 24) & 0xff);
    buffer[1] = (uint8_t) ((number >> 16) & 0xff);
    buffer[2] = (uint8_t) ((number >> 8) & 0xff);
    buffer[3] = (uint8_t) ((number >> 0) & 0xff);
}

uint8_t PlcCommand::getStringLength() const {
    const uint8_t lengthWithoutParam = 8 + framingLength;
    return (uint8_t) (lengthWithoutParam + payload.parameterLength);
}

/***
 * This methods replaces special characters 0x7E and 0x7D within the output buffer of an PLC command.
 * Here the end characters are ored with 0X20 and there is 0x7D in front of the changed one characters inserted.
*/
void PlcCommand::check_HDLCsequence() {
    enum {
        crcLength = 4,
        escapeSequence1 = 0x7E,
        escapeSequence2 = 0x7D,
        xorTerm = 0x20,
        escapeCharLength = 1
    };
    uint16_t length = getStringLength();
    uint32_t j = 0;
    for (uint32_t i = length - 2; i > 1; i--) {  // crcLength+escapeCharLength
        uint8_t selection = outputBuffer[i];
        if ((selection == escapeSequence1) || (selection == escapeSequence2)) {
            memmove(outputBuffer + (i + 2), outputBuffer + (i + 1), j);
            outputBuffer[i] = escapeSequence2;
            outputBuffer[i + 1] = selection ^ xorTerm;
            length++;
            payload.parameterLength++;
            j++;
        }
        j++;
    }
    outputBuffer[length - 1] = framingCode;
}

/***
* After receiving a character string, this method transfers the escape characters back into the RAW data string
* before sending. Compare with check_HDLCsequence()
* @param rxStream   - pointer to receiving string. It will be modified if necessary
* @param length     - length of the receiving string
*/
void PlcCommand::rollBack_HDLCSequence(char *rxStream, int16_t *length) {
    enum {
        escapeSequence1 = 0x7E, escapeSequence2 = 0x7D, xorTerm = 0x20
    };
    uint32_t lenCopy = *length;
    if (lenCopy < 2) {
        return;
    }
    rxStream++;
    for (uint32_t i = 1; i < lenCopy - 2; i++) {
        if (*rxStream == escapeSequence2) {
            char selection = *(rxStream + 1);
            if (selection == (escapeSequence2 ^ xorTerm)) {
                // 0x7D 0x5D -> 0x7D
                memmove(rxStream + 1, rxStream + 2, (lenCopy - i) - 1);
                *rxStream = escapeSequence2;
                lenCopy--;
            } else if (selection == (escapeSequence1 ^ xorTerm)) {
                // 0x7D 0x5E -> 0x7E
                memmove(rxStream + 1, rxStream + 2, (lenCopy - i) - 1);
                *rxStream = escapeSequence1;
                lenCopy--;
            }
        }
        rxStream++;
    }
    *length = (int16_t) lenCopy;
}

uint8_t PlcCommand::getDataMessageLength() const {
    const uint8_t lengthWithoutParam = 8 + framingLength;
    uint8_t strStartOffset = lengthWithoutParam + UDPFrame::ipv6HeaderLength
            + sizeof(UDPFrame::UDPformat);
    return (uint8_t) (this->payload.parameterLength - strStartOffset + 5);  // DS: +5 necessary correction value
}

void PlcCommand::copyNBytesFromArrayToParameter(uint32_t n, uint8_t *array) {
    if (n < kParamContainerLength + 1) {
        payload.parameterLength = (uint8_t) n;
        memcpy(payload.parameter, array, n);
    }
    /*      else{
     throw "Array to Long";
     } */
}

}  // namespace AME_SRC

