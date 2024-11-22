/*
 * PlcCommand.h
 *
 *  Created on: 16.02.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_PLCCOMMAND_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_PLCCOMMAND_H_

#include <cstring>
#include <cstdint>

#include "../PlcCommandTypeSpecification.h"

class exception;
namespace AME_SRC {

class G3Controller;
using namespace PLC_CommandTypeSpecification;

class PlcCommand {
 public:
    PlcCommand(PLCBlock blockType, CHANNEL channel, ACESSTyp accessTyp,
               LAYER accessLayer, uint8_t command, uint8_t *param,
               uint8_t paramLength);
    PlcCommand();
    explicit PlcCommand(char *buffer);
    PlcCommand(const PlcCommand &cmd);

    ~PlcCommand();
    char* toCommandStringHead() const;
    char* toCommandStringData(uint8_t partNr, bool *done) const;
//  char* toString();
    char* toRawString() const;
    char* toRawString2();

    uint8_t getDataMessageLength() const;
    uint8_t getStringLength() const;
    void calculateCRC();
    CHANNEL getChannel() const;
    ACESSTyp getAccesType() const;
    LAYER getAccesLayer() const;
    uint8_t getCommandID() const;

    static uint16_t getSrcIPFromDataCMD(const PlcCommand &cmd);

    void setReserveByte(uint8_t byte) {
        header.reserve = byte;
    }
    const uint8_t* getParameter() const;

    static const uint8_t framingCode = 0x7E;
    enum eSize { kBufSize = 176};  // ToDo(AME): keep size of outputBuffer in a useful relation to
                                   // ModemController::nsduContainerSize
    void copyNBytesFromArrayToParameter(uint32_t n, uint8_t *array);
    void check_HDLCsequence();
    static void rollBack_HDLCSequence(char *rxStream, int16_t *length);

    const char* getMessagePtr() const {
        return messagePtr;
    }

    void setMessagePtr(const char *_messagePtr_) {
        this->messagePtr = _messagePtr_;
    }

    uint8_t getConfirmIndex() const {
        return confirmIndex;
    }

    void setConfirmIndex(uint8_t _confirmIndex_) {
        this->confirmIndex = _confirmIndex_;
    }

    uint8_t getParameterLength() const {
        return payload.parameterLength;
    }

    void incCmdNr() {
        cmdNr = ++nextCmdNr;
    }
    uint8_t getCmdNr() const {
        return cmdNr;
    }

 private:
    const char *messagePtr;
    char* trimCommand(char *buffer);
    void writeUInt32ToBufferBigEndian(uint32_t number, uint8_t *buffer) const;
    void convertByteToFirstPaylodPart(uint8_t firstPayloadPart);
    static const uint8_t kParamContainerLength = 176;  // ToDo(AME): keep size into a useful relation to
                                                       // ModemController::nsduContainerSize geeignet angleichen
    struct Header {
        uint8_t reserve;
        uint8_t plcBlockType;
    } header;
    struct Payload {
        uint8_t channelID :1;
        uint8_t acessTyp :3;
        uint8_t targetLayer :4;
        uint8_t command;
        uint8_t parameterLength;
        uint8_t parameter[kParamContainerLength];
    } payload;
    uint32_t crc;
    uint8_t confirmIndex;
    uint8_t cmdNr;
    static uint8_t nextCmdNr;
    static uint8_t outputBuffer[];
    static const uint8_t framingLength = 2;

 public:
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_PLCCOMMAND_H_
