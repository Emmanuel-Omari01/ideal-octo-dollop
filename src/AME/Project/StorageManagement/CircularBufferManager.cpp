/*
 * CircularBufferManager.cpp
 *
 *  Created on: 27.02.2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "CircularBufferManager.h"
#include <string.h>
#include "../SystemBase/WatchDog/WatchDog.h"
#include "../HelpStructures/Uint8Operations.h"
#include "RenFlashApiAdapter.h"

#include "../StorageManagement/FlashData.h"

namespace AME_SRC {

uint8_t CircularBufferManager::stagingArea[32] = {0};
uint16_t CircularBufferManager::memoryWriteOffset = 0;
bool CircularBufferManager::memWriteInit = false;
bool CircularBufferManager::flashDeleted = false;

// Offsets of Container in memory
// define OFS_PREFIX 0
#define OFS_LEN     1
#define OFS_KEY     2
#define OFS_FLAGS   3
#define ADD_LEN     3       // 1 Byte prefix + 1 Byte data length field + 1 Byte Flags

// Structure in flash-Prom:
// "simple" key:
// Byte
// 0    1   2   3   4  ... 4+Data.length
// AA   ll  kk  FF  D0 ... Dn
// |    |   |   |   |       |
// |    |   |   |   |       +- letztes Daten-Bytes
// |    |   |   |   +--------- 1st data
// |    |   |   +------------- 8 Bits flags. Bit 0: 1=Block invalid
// |    |   +----------------- key kk In the range of 0x00.0x7F
// |    +--------------------- Data.length
// +-------------------------- prefix Data Flash Block

// "extended" key:
//  Byte
//  0   1   2   3   4   5   6  ... 6+Data.length
//  AA  ll  KK  FF  I0  I1  D0 ... Dn
//  |   |   |   |   |   |   |       |
//  |   |   |   |   |   |   |       +- letztes Daten-Bytes
//  |   |   |   |   |   |   +--------- 1. Datum
//  |   |   |   |   |   +------------- Index-Wort, MSB
//  |   |   |   |   +----------------- Index-Wort, LSB
//  |   |   |   +--------------------- 8 Bits Flags. Bit 0: 1=Block invalid
//  |   |   +------------------------- Schlüssel KK im Bereich von 0x80.0xFF
//  |   +----------------------------- Data.length
//  +--------------------------------- prefix Data Flash Block

CircularBufferManager::CircularBufferManager() {
}

CircularBufferManager::~CircularBufferManager() {
    // TODO(AME) Auto-generated destructor stub
}

// Write to circular buffer
// Return: True = Write successful
bool CircularBufferManager::storeKeyContainer(skey key, container val) {
    if (!memWriteInit) {
        setNextContainerAdr();
    }
//  uint8_t offset =  8-((val.length+4) % 8);
    uint16_t eff_len = (uint16_t) (val.length + getKeyLen(key) + ADD_LEN);
            // val.length+key_length+(prefix_length + Data_len_length + Flags_length);
    eff_len = align_len(eff_len);
    if (eff_len > sizeof(stagingArea)) {
        return false;
    }

    memset(stagingArea, 0, sizeof(stagingArea));
    stagingArea[0] = containerPrefix;
    stagingArea[OFS_LEN] = val.length;
    uint8_t *pMem = storeKey(stagingArea + OFS_KEY, key);  // +2..[+2|+3]
    memcpy(pMem, val.data, val.length);

    memoryWriteOffset = prepareFlashWrite(memoryWriteOffset, (uint8_t) eff_len);
    bool status = false;
    int tries = 6;
    while (status == false && (tries-- > 0)) {
        status = R_FLASH_Write((uint32_t) stagingArea,
                FLASH_DF_BLOCK_0 + memoryWriteOffset, eff_len) == FLASH_SUCCESS;
    }

    memoryWriteOffset = (uint16_t) (memoryWriteOffset + eff_len);
    if (memoryWriteOffset > (FLASH_DF_BLOCK_INVALID - FLASH_DF_BLOCK_0)) {
        memoryWriteOffset = 0;  // Offset of FLASH_DF_BLOCK_0;
    }
    return status;
}

void CircularBufferManager::loadContainerWithKey(skey* key, IKeyContainerStorage::container *cntnr) {
    if (!memWriteInit) {
        setNextContainerAdr();
    }

    cntnr->data = NULL;
    cntnr->length = 0;
    skey temp_key;
    uint8_t *searchIndex;
    uint8_t *lastIndex = 0;
    uint16_t con_len;           // container.length
    uint16_t key_len;
    uint32_t loops = 2;
    flash_err_t ret = FLASH_ERR_FAILURE;
    flash_res_t result = FLASH_RES_NOT_BLANK;
    uint32_t wAdr = FLASH_DF_BLOCK_0 + memoryWriteOffset;
    wAdr &= 0xFFFFFFFF ^ (FLASH_DF_BLOCK_SIZE - 1);
    searchIndex = reinterpret_cast<uint8_t*>(wAdr & 0xFFFFFFF8);
    while (loops) {     // searchIndex[0] != 0xFF
        ret = AME_SRC::R_FLASH_BlankCheck((uint32_t) searchIndex, (uint32_t) FLASH_DF_MIN_PGM_SIZE, &result);
        if ((ret == FLASH_SUCCESS) && (result == FLASH_RES_BLANK)) {  // && (((uint32_t) searchIndex & 0x07) == 0)
            searchIndex += 8;
            if (lastIndex)  // minimum 1 container found?
                break;
            goto test_loop;
        }
        if ((*searchIndex == containerPrefix)) {  // (((uint32_t) searchIndex & 0x07) == 0) &&
            temp_key.simple_key.key = *(searchIndex + OFS_KEY);
            key_len = getKeyLen(temp_key);
            con_len = (uint16_t) (*(searchIndex + OFS_LEN) + key_len + ADD_LEN);
            if (compareKey(searchIndex + OFS_KEY, *key)) {
                lastIndex = searchIndex;
            }
            searchIndex += align_len(con_len);
        } else {
            searchIndex += 8;   // ++
        }
        test_loop: if ((uint32_t) searchIndex >= FLASH_DF_BLOCK_INVALID) {
            searchIndex = reinterpret_cast<uint8_t*>(FLASH_DF_BLOCK_0);
            loops--;
        }
    }
    if (lastIndex) {
        loadContainerAtAdr(lastIndex, key, cntnr);
        return;
    }
    (*key).simple_key.flag.bit.invalid = 1;
}

// Loads data from a container
// memIdx: address to the containerPrefix
void CircularBufferManager::loadContainerAtAdr(uint8_t *memIdx, skey *key,
                                               IKeyContainerStorage::container *cntnr) {
    cntnr->data = NULL;
    cntnr->length = 0;

    if (*memIdx == containerPrefix) {
        uint8_t con_len = *(memIdx + OFS_LEN);
        skey temp_key;
        temp_key.simple_key.key = *(memIdx + OFS_KEY);
        uint16_t key_len = getKeyLen(temp_key);
        uint16_t data_ofs = (uint16_t) (key_len + ADD_LEN);
        if (con_len + data_ofs < sizeof(stagingArea)) {
            cntnr->data = memIdx + data_ofs;
            cntnr->length = con_len;
            (*key).simple_key.flag.all = *(memIdx + OFS_FLAGS);
        }
    }
}

// Compare 2 containers with each other
bool CircularBufferManager::compare2Container(container con1, container con2) {
    if ((con1.data == 0) || (con2.data == 0)) {
        return false;
    }
    if (con1.length != con2.length) {
        return false;
    }
    return static_cast<bool>(memcmp(con1.data, con2.data, con1.length) == 0);
}

// Compare the flags of 2 key fields with each other
bool CircularBufferManager::compare2Flags(skey key1, skey key2) {
    return key1.simple_key.flag.all == key2.simple_key.flag.all;
}

// Search start address of the last used container
// Return:
// return, uint16_t - start address of the last container (offset in Flash-Prom)
// nextIndex - Offset to the next block in Flash-Prom
uint16_t CircularBufferManager::searchLastContainer(uint16_t *nextIndex) {
    uint8_t *searchIndex = reinterpret_cast<uint8_t*>(FLASH_DF_BLOCK_0);
    uint8_t *lastIndex = searchIndex;
    flash_err_t ret = FLASH_ERR_FAILURE;    // Declare flash API error flag
    flash_res_t result = FLASH_RES_NOT_BLANK;
    uint16_t con_len;       // container.length
    while (true) {
        // check whether the memory to be examined is empty
        ret = AME_SRC::R_FLASH_BlankCheck((uint32_t) searchIndex, FLASH_DF_MIN_PGM_SIZE, &result);
        if ((ret == FLASH_SUCCESS) && (result == FLASH_RES_BLANK)) {
            break;  // The following containers are older or not occupied
        }

        if (*searchIndex == containerPrefix) {
            lastIndex = searchIndex;
            con_len = *(searchIndex + OFS_LEN);
            skey temp_key;
            temp_key.simple_key.key = *(searchIndex + OFS_KEY);
            uint16_t key_len = getKeyLen(temp_key);
            con_len = (uint16_t) (con_len + key_len + ADD_LEN);
            searchIndex += align_len(con_len);
        } else {
            searchIndex += 8;
        }
        if (((uint32_t) searchIndex) >= FLASH_DF_BLOCK_INVALID) {
            searchIndex = reinterpret_cast<uint8_t*>(FLASH_DF_BLOCK_0);
            break;
        }
    }
    uint8_t *tmpIndex = searchIndex - FLASH_DF_BLOCK_0;
    *nextIndex = (uint16_t) ((uint32_t) tmpIndex);
    lastIndex -= FLASH_DF_BLOCK_0;
    return (uint16_t) ((uint32_t) lastIndex);
}

// Reset writing pointer to the next address after the last container
void CircularBufferManager::setNextContainerAdr() {
    container tmp;
    uint16_t nextOfs;
    uint16_t lastOfs = searchLastContainer(&nextOfs);
    skey temp_key;

    loadContainerAtAdr(reinterpret_cast<uint8_t*>(FLASH_DF_BLOCK_0 + lastOfs), &temp_key, &tmp);
    if (tmp.data) {
        lastOfs = nextOfs;
    }
    memoryWriteOffset = lastOfs;
    memWriteInit = true;
}

// Determines the length of the key field
uint16_t CircularBufferManager::getKeyLen(skey key) {
    return (key.simple_key.key & 0x80)? 3:1;
}

// Set key in the writing buffer.
// Input:  pMem - pointer to the first byte in the key field
//         key  - key which should be written
// Return: pointer to the first data
uint8_t* CircularBufferManager::storeKey(uint8_t *pMem, skey key) {
    *pMem++ = key.simple_key.key;
    *pMem++ = key.simple_key.flag.all;
    if (key.simple_key.key & 0x80) {  // "Flag" für key mit Index-Feld gesetzt?
        *(reinterpret_cast<uint16_t*>(pMem)) = key.extended_key.index;
        pMem += 2;
    }
    return pMem;
}

// Compare keys in the write buffer with a passed key
bool CircularBufferManager::compareKey(uint8_t* pMem, skey key) {
    if (*pMem++ != key.simple_key.key) {
        return false;
    }
    // if (*pMem++ != key.simple_key.flag.all) {
    //  return false;
    // }
    pMem++;
    if (key.simple_key.key & 0x80) {  // "Flag" set for key in the index field?
        return *(reinterpret_cast<uint16_t*>(pMem)) == key.extended_key.index;
    }
    return true;
}

// Do Alignment for the container length: Round up to multiples of 8
uint16_t CircularBufferManager::align_len(uint16_t check_len) {
    if (check_len % 8) {
        check_len = (uint16_t) (check_len + 8u - (check_len % 8u));
    }
    return check_len;
}

// Preparing to write in Flash Prom. Delete block if necessary
// Return: wOfs, optional has new value.
uint16_t CircularBufferManager::prepareFlashWrite(uint16_t wOfs, uint8_t wlen) {
    uint32_t wAdr = FLASH_DF_BLOCK_0 + wOfs;
    uint32_t wAdr2, DF_MASK, i;
    flash_err_t ret = FLASH_ERR_FAILURE;    // Declare flash API error flag
    flash_res_t result = FLASH_RES_NOT_BLANK;
    bool do_erase;
    DF_MASK = 0xFFFFFFFF ^ (FLASH_DF_BLOCK_SIZE - 1);

    for (i = 1; i <= 2; i++) {
        // ret = R_FLASH_BlankCheck(wAdr, FLASH_DF_MIN_PGM_SIZE, &result);
        ret = AME_SRC::R_FLASH_BlankCheck(wAdr, FLASH_DF_MIN_PGM_SIZE, &result);
        switch (ret) {
        case FLASH_ERR_BYTES:
        case FLASH_ERR_ADDRESS:
            do_erase = true;
            wAdr = FLASH_DF_BLOCK_0;
            wOfs = 0;
            break;
        default:
            do_erase = static_cast<bool>((ret != FLASH_SUCCESS) || (result != FLASH_RES_BLANK));
            break;
        }
        if (do_erase) {
            // Erase_FlashData(wAdr & DF_MASK);
            AME_SRC::R_FLASH_Erase((flash_block_address_t) (wAdr & DF_MASK), 1);
            flashDeleted = true;
        }
        wAdr2 = wAdr + wlen;
        // block limit Flash-Prom exceeded?
        if ((wAdr & DF_MASK) == (wAdr2 & DF_MASK)) {
            // no, same block
            break;
        }
        wAdr = wAdr2;
    }
    return wOfs;
}

bool CircularBufferManager::containerDeleted() {
    return flashDeleted;
}
void CircularBufferManager::releaseContainerDeleted() {
    flashDeleted = false;
}

}  // namespace AME_SRC
