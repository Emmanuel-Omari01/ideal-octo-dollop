/* Copyright (C) 2024 AME - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Tobias Hirsch <t.hirsch@amelectronic.de>
 */

#include "CodeFlashEditor.h"

#include "../StorageManagement/RenFlashApiAdapter.h"
#include "../SystemBase/InterruptController.h"

#include "../../../Renesas/Generated/r_bsp/mcu/all/r_rx_compiler.h"
#include "../../../Renesas/Generated/r_flash_rx/src/targets/rx111/r_flash_rx111.h"


namespace AME_SRC {

volatile uint8_t CodeFlashEditor::g_buf[FLASH_DF_BLOCK_SIZE];
CodeFlashEditor::CodeFlashEditor() :
        readyStatus_(false), vecCopyIndex_(0), storeStatus_(kFailure) {
  vectorTableStartAddress = &rvectors_start;
    setupStep = openDriver;
}

CodeFlashEditor::~CodeFlashEditor() {
    R_FLASH_Close();
}

void CodeFlashEditor::store(const uint8_t *data, uint32_t address, uint16_t size) {
    flash_err_t err = FLASH_ERR_BUSY;
    flash_res_t result;
    flash_block_address_t writeBlock = resolveUpdateAddress(address);

    this->clear(address, size);

    if (storeStatus_ == IStorage::kClearSuccess) {
        InterruptController::disable(InterruptController::all);
        err = AME_SRC::R_FLASH_Write((uint32_t) data, writeBlock, size);
        if (err != FLASH_SUCCESS) {
            storeStatus_ = IStorage::kFailure;
            return;
        }
        storeStatus_ = IStorage::kStoreSuccess;
        InterruptController::enable(InterruptController::all);
    }
}

void CodeFlashEditor::clear(uint32_t address, uint16_t size) {
    flash_err_t err = FLASH_ERR_BUSY;
    flash_res_t result;
    flash_block_address_t clearBlock = resolveUpdateAddress(address);

    InterruptController::disable(InterruptController::all);
    /* Erase code flash block */
    err = AME_SRC::R_FLASH_Erase(clearBlock, (uint32_t) 1);
    if (err != FLASH_SUCCESS) {
        InterruptController::enable(InterruptController::all);
        storeStatus_ = IStorage::kFailure;
        return;
    }

    /* Verify erased */
    err = AME_SRC::R_FLASH_BlankCheck(clearBlock,
    FLASH_CF_BLOCK_SIZE, &result);
    if ((err != FLASH_SUCCESS) || (result != FLASH_RES_BLANK)) {
        storeStatus_ = IStorage::kFailure;
    } else {
        storeStatus_ = IStorage::kClearSuccess;
    }
    InterruptController::enable(InterruptController::all);
}

void CodeFlashEditor::setup() {
    R_FLASH_Open();
    readyStatus_ = true;
}

bool CodeFlashEditor::isReady() {
    return readyStatus_;
}

IStorage::storeStates CodeFlashEditor::getStatus() {
    return storeStatus_;
}

flash_block_address_t CodeFlashEditor::resolveUpdateAddress(uint32_t address) {
    uint16_t blockSize = 0x400;
    uint16_t cfBlockAmount = 511;
    flash_block_address_t updateBlock = FLASH_DF_BLOCK_INVALID;
    if ((FLASH_CF_BLOCK_0 > address) && (address > FLASH_CF_BLOCK_511)) {
        uint32_t updateBlockIndex = (cfBlockAmount
                - (address - FLASH_CF_BLOCK_511) / blockSize);
        updateBlock = static_cast<flash_block_address_t>(FLASH_CF_BLOCK_0
                - (updateBlockIndex * blockSize));
    }
    return updateBlock;
}

}  // namespace AME_SRC
