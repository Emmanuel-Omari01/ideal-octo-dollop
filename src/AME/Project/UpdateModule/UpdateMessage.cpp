/*
 * UpdateMessage.cpp
 *
 *  Created on: Jun 27, 2024
 *      Author: plcdev
 */

#include "UpdateMessage.h"
#include <stdio.h>
#include "UpdateDependencies.h"

namespace AME_SRC {

UpdateMessage::UpdateMessage(UpdateType type, uint16_t id, uint16_t offset,
        uint16_t size, const uint8_t *storagePtr) :
        _type(type), _id(id), _offset(offset), _size(size), _storagePtr(
                storagePtr) {
}

UpdateMessage::UpdateMessage() :
        _type { UpdateType::error }, _id { 0 }, _offset { 0 }, _size { 0 }, _storagePtr {
                NULL } {
}

UpdateMessage::~UpdateMessage() {
}

const char* UpdateMessage::toRAW() {
    uint8_t messageHeaderLength = 19;
    snprintf(Global::OutBuff, messageHeaderLength, "::U:c%:%03u:%03u:%04d:,", _type, _id, _offset,
            _size);
    return Global::OutBuff;
}



} /* namespace AME_SRC */
