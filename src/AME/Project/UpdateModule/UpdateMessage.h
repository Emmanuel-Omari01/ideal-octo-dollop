/*
 * UpdateMessage.h
 *
 *  Created on: Jun 27, 2024
 *      Author: plcdev
 */

#ifndef SRC_AME_PROJECT_UPDATEMODULE_UPDATEMESSAGE_H_
#define SRC_AME_PROJECT_UPDATEMODULE_UPDATEMESSAGE_H_
#include <stdint.h>
namespace AME_SRC {
enum class UpdateType {
    fragment = 'f',
    start = 'S',
    startReply = 's',
    nack = 'n',
    ack = 'a',
    error = 'e'
};

class UpdateMessage {
 public:
    UpdateMessage();
    UpdateMessage(UpdateType type, uint16_t id, uint16_t offset, uint16_t size,
            const uint8_t *storagePtr);
    virtual ~UpdateMessage();

    const char* toRAW();

    void setStoragePtr(const uint8_t *storagePtr) {
        _storagePtr = storagePtr;
    }

    uint16_t getId() const {
        return _id;
    }

    uint16_t getOffset() const {
        return _offset;
    }

    uint16_t getSize() const {
        return _size;
    }

    const uint8_t* getStoragePtr() const {
        return _storagePtr;
    }

    UpdateType getType() const {
        return _type;
    }

    void setId(uint16_t id) {
        _id = id;
    }

    void setOffset(uint16_t offset) {
        _offset = offset;
    }

    void setSize(uint16_t size) {
        _size = size;
    }

    void setType(UpdateType type) {
        _type = type;
    }

    UpdateType _type;
    uint16_t _id;
    uint16_t _offset;
    uint16_t _size;
    const uint8_t *_storagePtr;
};

} /* namespace AME_SRC */

#endif /* SRC_AME_PROJECT_UPDATEMODULE_UPDATEMESSAGE_H_ */
