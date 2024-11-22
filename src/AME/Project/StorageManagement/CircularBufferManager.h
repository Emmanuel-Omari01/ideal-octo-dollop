/*
 * CircularBufferManager.h
 *
 *  Created on: 27.02.2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_STORAGEMANAGEMENT_CIRCULARBUFFERMANAGER_H_
#define SRC_AME_PROJECT_STORAGEMANAGEMENT_CIRCULARBUFFERMANAGER_H_

#include "IKeyContainerStorage.h"
#include "../SystemBase/Global.h"
#include "../../../Renesas/Generated/r_flash_rx/r_flash_rx_if.h"
#include "../../../Renesas/Generated/r_flash_rx/src/targets/rx111/r_flash_rx111.h"

namespace AME_SRC {

class CircularBufferManager: public IKeyContainerStorage {
 public:
    CircularBufferManager();
    virtual ~CircularBufferManager();
    bool storeKeyContainer(skey key, container val);
    void loadContainerWithKey(skey *key, container *cntnr);
    bool compare2Container(container con1, container con2);
    bool compare2Flags(skey key1, skey key2);
    void setNextContainerAdr();
    uint16_t align_len(uint16_t check_len);
    uint16_t prepareFlashWrite(uint16_t wOfs, uint8_t wlen);
    bool containerDeleted();
    void releaseContainerDeleted();
    enum eCBParameter {
        ExtendedHeaderLen = 6,
        MaxDataLen = 26,
        StageingAreaSize = ExtendedHeaderLen + MaxDataLen,
    };

 private:
    uint16_t getKeyLen(skey key);
    uint8_t* storeKey(uint8_t *pMem, skey key);
    bool compareKey(uint8_t *pMem, skey key);
    void loadContainerAtAdr(uint8_t *memIdx, skey *key, container *cntnr);
    uint16_t searchLastContainer(uint16_t *nextIndex);
    static uint8_t stagingArea[StageingAreaSize];
    static uint16_t memoryWriteOffset;
    static bool memWriteInit;
    static bool flashDeleted;
    enum {
        containerPrefix = 0xAA
    };
    // const uint16_t containerPrefix = 0xAA;
};

extern "C" flash_err_t R_FLASH_Write(uint32_t src_address,
        uint32_t dest_address, uint32_t num_bytes);
extern "C" flash_err_t R_FLASH_Erase(flash_block_address_t block_start_address,
        uint32_t num_blocks);

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_STORAGEMANAGEMENT_CIRCULARBUFFERMANAGER_H_
