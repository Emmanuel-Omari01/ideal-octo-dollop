/*
 * SimpleFlashEditor.cpp
 *
 *  Created on: 21.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "SimpleFlashEditor.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "../SystemBase/Global2.h"
#include "../SystemBase/WatchDog/WatchDog.h"
#include "../SystemLogic/StorageAdministrator.h"
#include "../SystemLogic/I18N.h"
#include "../StorageManagement/CircularBufferManager.h"
#include "../HelpStructures/CharOperations.h"

namespace AME_SRC {

// Imported global variable, list of block addresses array
const uint32_t g_flash_BlockAddresses[8] = { FLASH_DF_BLOCK_0, FLASH_DF_BLOCK_1,
        FLASH_DF_BLOCK_2, FLASH_DF_BLOCK_3, FLASH_DF_BLOCK_4, FLASH_DF_BLOCK_5,
        FLASH_DF_BLOCK_6, FLASH_DF_BLOCK_7 };  // DB7

SimpleFlashEditor::Parameter_A SimpleFlashEditor::flash_A;
SimpleFlashEditor::Parameter_FC SimpleFlashEditor::flash_FC;
static bool read_A, read_FC;

SimpleFlashEditor::SimpleFlashEditor() {
    read_A = false;
    read_FC = false;
}

/***
 bool simpleFlashEditor::writeFlashData(uint8_t block_nr) {
 void *data;
 U16 size;
 switch (block_nr) {
 case BLOCK_RT:
 data = &flash_RT;
 size = sizeof(Parameter_RT);
 break;
 default:
 // data = &flash_A;
 // size = sizeof(Parameter_A);
 return false;
 }
 *((U16 *) data) = ID_FLASH;	// Record-ID vorbesetzen
 return  ame_flash_write_record(data, block_nr, size, ID_FLASH);
 }
 ***/

uint32_t SimpleFlashEditor::getFrameCounter() {
    return flash_FC.FrameCounter;
}

void SimpleFlashEditor::setFrameCounter(uint32_t unsignedShortInt) {
    flash_FC.FrameCounter = unsignedShortInt;
}

bool SimpleFlashEditor::readFlashData() {
    if (!read_A) {
        readParamFlash();
    }
    if (!read_FC) {
        readFCFlash();
    }
    return read_A;  //  & read_FC // DS: frame counter is not written any more
}

bool SimpleFlashEditor::readParamFlash() {
    if (readFlashContainer(StorageAdministrator::sysParameter, &flash_A,
            sizeof(flash_A))) {
        read_A = true;
    }
    return read_A;
}

bool SimpleFlashEditor::readFCFlash() {
    if (readFlashContainer(StorageAdministrator::frameCounter, &flash_FC,
            sizeof(flash_FC))) {
        read_FC = true;
    }
    return read_FC;
}

bool SimpleFlashEditor::readFlashContainer(int key, void *dest,
        uint32_t destLen) {
    IKeyContainerStorage *iFace =
            StorageAdministrator::getStorageInterfacePtr();
    IKeyContainerStorage::skey temp_key;
    temp_key.simple_key.key = (uint8_t) key;
    IKeyContainerStorage::container loadedCon;
    iFace->loadContainerWithKey(&temp_key, &loadedCon);
    if (loadedCon.data && destLen <= loadedCon.length) {
        memcpy(dest, loadedCon.data, loadedCon.length);
        return true;
    } else {
        return false;
    }
}

/*** DS, AME: 22.05.2024 obsolet
 bool SimpleFlashEditor::writeFCFlash() {
 bool result;
 IKeyContainerStorage *iFace = StorageAdministrator::getStorageInterfacePtr();
 IKeyContainerStorage::skey temp_key;
 temp_key.simple_key.key = StorageAdministrator::frameCounter;
 temp_key.simple_key.flag.all = 0;
 result = iFace->storeKeyValPair(temp_key, reinterpret_cast<uint8_t *>(&flash_FC),
 sizeof(Parameter_FC));
 if (iFace->containerDeleted()) {
 updateSysParamFlash();
 iFace->releaseContainerDeleted();
 }
 return result;
 }
 ***/

void SimpleFlashEditor::updateSysParamFlash() {
    IKeyContainerStorage *iFace =
            StorageAdministrator::getStorageInterfacePtr();
    IKeyContainerStorage::container saveParam = {0, 0 };
    saveParam.data = reinterpret_cast<const uint8_t*>(&flash_A);
    saveParam.length = sizeof(Parameter_A);
    IKeyContainerStorage::skey temp_key, save_key;
    temp_key.simple_key.key = StorageAdministrator::sysParameter;
    temp_key.simple_key.flag.all = 0;
    save_key = temp_key;
    IKeyContainerStorage::container loadedParam;
    iFace->loadContainerWithKey(&save_key, &loadedParam);
    if (!iFace->compare2Container(loadedParam, saveParam)
            || !iFace->compare2Flags(temp_key, save_key)) {
        iFace->storeKeyValPair(temp_key, reinterpret_cast<uint8_t*>(&flash_A),
                sizeof(Parameter_A));
    }
}

char* SimpleFlashEditor::setDefaultSysParameter(__attribute__((unused)) char *dummy) {
    preSetFlashA(false);
    updateSysParamFlash();
    return const_cast<char *>(TX::getText(TX::DefaultParametersSet));
}

void SimpleFlashEditor::setSysParamID(uint16_t id) {
    flash_A.board_ID = id;
    updateSysParamFlash();
}

void SimpleFlashEditor::setAutoStartFlowMode(uint16_t mode) {
    flash_A.autoStartFlowMode = mode;
    updateSysParamFlash();
}
void SimpleFlashEditor::setTargetNetId(uint16_t net_id) {
    flash_A.targetNetId = net_id;
    updateSysParamFlash();
}
void SimpleFlashEditor::setOwnNetworkAddress(uint16_t adr) {
    flash_A.ownNetworkAddress = adr;
    updateSysParamFlash();
}
void SimpleFlashEditor::setFJCallout(uint16_t ca) {
    flash_A.fjCallouts = ca;
    updateSysParamFlash();
}
void SimpleFlashEditor::setFJDelay(uint8_t delay, uint16_t index) {
    if (index < sizeof(flash_A.fjDelay)) {
        flash_A.fjDelay[index] = delay;
        updateSysParamFlash();
    }
}
void SimpleFlashEditor::setFJScale(uint16_t scale) {
    flash_A.fjScale = scale;
    updateSysParamFlash();
}

SimpleFlashEditor::~SimpleFlashEditor() = default;

void SimpleFlashEditor::initFlash() {
    R_FLASH_Open();
}

char* SimpleFlashEditor::eraseDataFlash(char *param_term) {
    U32 block = FLASH_DF_BLOCK_0;
    const char *prm_trm = charOperations::skip_token(param_term, ' ');
    if (*prm_trm == 'A') {  // 'A'lle?
        block = 0xFFFFFFFF;
    }
    if (*prm_trm >= '0' && *prm_trm <= '7') {  // 0..7
        block = *prm_trm - '0';
    }
    while (Erase_FlashData(block) != FLASH_SUCCESS) {
        WatchDog::feed();
    }
    return const_cast<char *>(TX::getText(TX::cTxtFlashBlockDelMsg));
}

/*******************************************************************************
 * Outline       : Erase_FlashData
 * Description   : This function enters a for loop, and erases a block of data
 *                 flash memory each iteration until all blocks have been erased.
 * Argument      : user_adr: 0...7 Block-Nr des zu löschenden Blocks
 *                           0xFFFFFFFF: alle Blöcke
 *                           ?           Adresse des zu löschenden Blocks
 * Return value  : Rückmeldung von
 *         R_FlashErase          FLASH_SUCCESS (0), FLASH_FAILURE (6), FLASH_BUSY (5) oder
 *         R_FlashDataAreaBlankCheck FLASH_BLANK (0), FLASH_NOT_BLANK (1), FLASH_FAILURE (6), FLASH_BUSY (5), FLASH_ERROR_ADDRESS (3), FLASH_ERROR_BYTES(2)
 *******************************************************************************/
flash_err_t SimpleFlashEditor::Erase_FlashData(U32 user_block) {
    flash_err_t err;    // Declare flash API error flag
    if (user_block < 8) {
        err = AME_SRC::R_FLASH_Erase(
                (flash_block_address_t) g_flash_BlockAddresses[user_block], 1);  // , FLASH_NUM_BLOCKS_DF
    } else if (user_block == 0xFFFFFFFF) {
        err = AME_SRC::R_FLASH_Erase(FLASH_DF_BLOCK_0, FLASH_NUM_BLOCKS_DF);
    } else {
        err = AME_SRC::R_FLASH_Erase((flash_block_address_t) user_block, 1);
    }

    return err;
}

char* SimpleFlashEditor::memoryWriteTest(char *value) {
    IKeyContainerStorage *keyValueStore = StorageAdministrator::getStorageInterfacePtr();
    char *textPtr, *savePtr;
    textPtr = strtok_r(value, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    var32 key;
    key.all = atoi(textPtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    uint8_t len = (uint8_t) strlen(textPtr);
    IKeyContainerStorage::skey temp_key;
    temp_key.all = 0;
    if (key.all < 0x80) {
        temp_key.simple_key.key = key.bx[0];
    } else {
        temp_key.extended_key.key = key.bx[2];
        temp_key.extended_key.index = key.word.lw;
    }
    keyValueStore->storeKeyValPair(temp_key, reinterpret_cast<uint8_t*>(textPtr), len);
    if (keyValueStore->containerDeleted()) {
        updateSysParamFlash();
        keyValueStore->releaseContainerDeleted();
    }
    // uint16_t data = atoi(textPtr);
    //// while (((keyValueStore->storeKeyValPair(2, data))== false) && (tries--)){}
    // keyValueStore->storeKeyValPair(key, data);
    return const_cast<char*>(TX::getText(TX::MemoryIsWritten));
}

char* SimpleFlashEditor::memoryReadTest(char *value) {
    // IKeyContainerStorage* keyValueStore = &CircularBufferManager();
    IKeyContainerStorage *keyValueStore = StorageAdministrator::getStorageInterfacePtr();
    char ckey[8];
    char *textPtr, *savePtr;
    textPtr = strtok_r(value, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    var32 key;
    key.all = atoi(textPtr);
    IKeyContainerStorage::skey temp_key;
    if (key.all < 0x80) {
        temp_key.simple_key.key = key.bx[0];
        ckey[0] = '-';
        ckey[1] = '-';
        ckey[2] = 0;    // sprintf(ckey,"%s","--");
    } else {
        temp_key.extended_key.key = key.bx[2];
        temp_key.extended_key.index = key.word.lw;
        snprintf(ckey, sizeof(ckey), "%02x", temp_key.extended_key.index);
    }
    // while (((keyValueStore->storeKeyValPair(2, data))== false) && (tries--)){}
    IKeyContainerStorage::container tmp;
    keyValueStore->loadContainerWithKey(&temp_key, &tmp);
    // 0123456789012345678901234567890123456789012
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "Adr:%08x Key:%02x Idx:%s Invalid:%1d Data: ",
            (unsigned int) tmp.data, temp_key.simple_key.key, ckey,
            temp_key.simple_key.flag.bit.invalid);  // Adr:00000000 Key:xx Idx:yy Invalid:? Data:
    for (int i = 0; i < tmp.length; i++) {
        uint32_t idx = 42 + i * 2;
        snprintf(reinterpret_cast<char *>(&Global2::OutBuff[idx]), Global2::outBuffMaxLength - idx,
                "%02X", tmp.data[i]);
    }
    return Global2::OutBuff;
}

void SimpleFlashEditor::store(uint8_t *data, uint32_t adress, uint16_t size) {
}

}  // namespace AME_SRC
