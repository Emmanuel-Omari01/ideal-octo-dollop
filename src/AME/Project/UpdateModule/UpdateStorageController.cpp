/*
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "UpdateStorageController.h"

namespace AME_SRC {

UpdateStorageController::UpdateStorageController(IStorage *solidStorage) :
        _solidStorage { solidStorage } {
}

void UpdateStorageController::storeFragment(
        UpdateMessage uMessage, uint32_t adress) {
    _solidStorage->store(uMessage.getStoragePtr(), adress, uMessage.getSize());
}

IStorage::storeStates UpdateStorageController::getStatus() {
    return _solidStorage->getStatus();
}

UpdateStorageController::~UpdateStorageController() {}

} /* namespace AME_SRC */
