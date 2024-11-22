/* Copyright (C) 2024 Tobias Hirsch - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the XYZ license.
 */

#include "PlcUpdateTranslator.h"

#include <cstdio>

namespace AME_SRC {


PlcUpdateTranslator::PlcUpdateTranslator() {
            _isMessageValid = false;
    _uMessage = UpdateMessage();
}

PlcUpdateTranslator::~PlcUpdateTranslator() {
}

UpdateMessage& PlcUpdateTranslator::getNextTranslation() {
    if (!updateQueue.isEmpty()) {
        _uMessage = updateQueue.dequeue();
        return _uMessage;
    } else {
        _uMessage = { UpdateType::error, 0, 0, 0, 0 };
        return _uMessage;
    }
}

bool PlcUpdateTranslator::isUpdateMessage() {
    return _isMessageValid;
}

UpdateMessage& PlcUpdateTranslator::translateMessage(
        const char *message) {
    char prefix, type = 0;
    unsigned int cnt = sscanf(message, "%c:%c:%02hu:%02hu:%03hu:%c", &prefix,
            type, _uMessage.getId(), _uMessage.getOffset(), _uMessage.getSize(),
            _uMessage.getStoragePtr());
    if (cnt == 6) {
        if ((prefix == getUpdatePrefix()) && isUpdateTypeValid(type)) {
            _uMessage.setType(static_cast<UpdateType>(type));
            _isMessageValid = true;
        } else {
            _uMessage.setType(UpdateType::error);
            _uMessage.setId(0);
            _uMessage.setOffset(0);
            _uMessage.setSize(0);
            _uMessage.setStoragePtr(0);
        }
    }
    return _uMessage;
}

void PlcUpdateTranslator::storeMessage(const UpdateMessage& uMessage) {
    updateQueue.enqueue(uMessage);
}
}  // namespace AME_SRC
