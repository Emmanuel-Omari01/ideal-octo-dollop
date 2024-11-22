/* Copyright (C) 2024 AME - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Tobias Hirsch <t.hirsch@amelectronic.de>
 */

#ifndef SRC_AME_PROJECT_UPDATEMODULE_CODEFLASHEDITOR_H_
#define SRC_AME_PROJECT_UPDATEMODULE_CODEFLASHEDITOR_H_

#include <cinttypes>

#include "../StorageManagement/RenFlashApiAdapter.h"
#include "../StorageManagement/IStorage.h"

#include "../../../Renesas/Generated/r_flash_rx/src/targets/rx111/r_flash_rx111.h"

namespace AME_SRC {

class CodeFlashEditor: public IStorage {
 public:
    CodeFlashEditor();
    virtual ~CodeFlashEditor();
    void store(const uint8_t *data, uint32_t address, uint16_t size) override;
    void clear(uint32_t address, uint16_t size) override;
    void setup() override;
    bool isReady() override;
    IStorage::storeStates getStatus() override;
 private:
    enum setupSteps {
        openDriver, copyVectorTable, finishSetup, idle
    } setupStep;

    flash_block_address_t resolveUpdateAddress(uint32_t address);
    static volatile uint8_t g_buf[FLASH_DF_BLOCK_SIZE];
    const void *vectorTableStartAddress;
    bool readyStatus_;
    uint32_t vecCopyIndex_;
    IStorage::storeStates storeStatus_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_UPDATEMODULE_CODEFLASHEDITOR_H_
