/*
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_UPDATEMODULE_UPDATESTORAGECONTROLLER_H_
#define SRC_AME_PROJECT_UPDATEMODULE_UPDATESTORAGECONTROLLER_H_

#include "IUpdateTranslator.h"

#include "../StorageManagement/IStorage.h"

namespace AME_SRC {


class UpdateStorageController {
 public:
    explicit UpdateStorageController(IStorage* solidStorage);
    void storeFragment(UpdateMessage uMessage,  uint32_t adress);
    IStorage::storeStates getStatus();
    virtual ~UpdateStorageController();
 private:
    IStorage* _solidStorage;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_UPDATEMODULE_UPDATESTORAGECONTROLLER_H_
