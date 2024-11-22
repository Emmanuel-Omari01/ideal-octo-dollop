/*
q * i2capp.cpp
 *
 *  Created on: May 14, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "I2cApp.h"

#include <cstring>
#include <cstdio>

#include "../../SystemBase/Global2.h"
#include "../../HelpStructures/CharOperations.h"
#include "../../SystemBase/ProgramConfig.h"

#include "../../../Renesas/NotGenerated/r_cg_lock.h"

namespace AME_SRC {

i2c_app::riic_return_t i2c_app::i2c_write_result;
i2c_app::riic_return_t i2c_app::i2c_read_result;
i2c_app::i2cFkn_t i2c_app::i2cFunction;
i2c_app::i2cSubFkn_t i2c_app::i2cSubFunction;

uint8_t i2c_app::i2cAdr[1];
uint8_t i2c_app::access_addr[2];
uint8_t i2c_app::ee_buf[128];
i2c_app::mac_info_t i2c_app::mac_info;

TermContextFilter *i2c_app::termFilterPtr_;

i2c_app::i2c_app(SerialDriver *term) {
    termFilterPtr_ = new TermContextFilter(term);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);

    i2c_read_result = RIIC_ERR_NO_INIT;
    i2c_write_result = RIIC_ERR_NO_INIT;
    i2cAdr[0] = macChipAdr;
    i2cFunction = FKN_IDLE;
    mac_info.status = RIIC_ERR_NO_INIT;

    memset(&access_addr, 0, sizeof(access_addr));

    // init timeout watcher object for short delays
    // Delay is needed between read and write operation
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    ITimeOutWatcher::timeOutInfo* pToi = pW->findTimeOutWithID(ProgramConfig::i2cTaskID);
    if (pToi == NULL) {
        ITimeOutWatcher::timeOutInfo toi;
        ITimeOutWatcher::createDefaultTimeOut(&toi, ProgramConfig::i2cTaskID, &i2cTimed50ms, NULL);
        toi.delay = 1;           // 50ms
        toi.timeCounter = 1;
        pW->registerTimeout(toi);
    }

    R_System_Unlock();
    R_Config_RIIC0_Create();
    R_System_Lock();

    R_Config_RIIC0_Start();
    R_Config_RIIC0_register_cb(error, &i2cDumpError);
    R_Config_RIIC0_register_cb(readend, &i2cReadEnd);
    R_Config_RIIC0_register_cb(writeend, &i2cWriteEnd);
}

i2c_app::~i2c_app() {
    // TODO(AME): Auto-generated destructor stub
}

char *i2c_app::i2c_init(__attribute__((unused)) char *text) {
    R_Config_RIIC0_Start();
    return const_cast<char *>("i2c bus init");
}

char *i2c_app::i2c_sample_write(char *text) {
    int16_t amount = terminalInput2Buffer(text);
    if (amount <= 0) {
        return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
    }

    // Write sample data once
    i2c_write_result = (riic_return_t) R_Config_RIIC0_Master_Send(i2cAdr[0], ee_buf, amount);  // RIIC send start
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            "Sample data are going to write. Return value: %x", i2c_write_result);
    return Global2::OutBuff;
}  // i2c_sample_write

char *i2c_app::i2c_sample_read(char *text) {
    int16_t amount = terminalInput2Buffer(text);
    if (amount <= 0) {
        return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
    }

    // Read sample data once
    i2c_read_result = (riic_return_t) R_Config_RIIC0_Master_Receive(i2cAdr[0], ee_buf, amount);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            "Sample data going to read. Return value: %x", i2c_read_result);
    return Global2::OutBuff;
}  // i2c_sample_read

char *i2c_app::i2cMacRead(__attribute__((unused)) char *text) {
    i2cFunction = FKN_MAC_READ;
    i2cSubFunction = SUB_FKN_WRITE;
    i2cTriggerTimer();
    mac_info.status = RIIC_ERR_NO_INIT;

    ee_buf[0] = macChipOfsEUI64;
    i2c_write_result = (riic_return_t) R_Config_RIIC0_Master_Send(macChipAdr, ee_buf, 1);  // RIIC send start

    return const_cast<char *>("");
}

void i2c_app::i2cTimed50ms() {
    if (i2cFunction == FKN_MAC_READ) {
        switch (i2cSubFunction) {
            case SUB_FKN_WRITE:
                i2c_read_result = (riic_return_t) R_Config_RIIC0_Master_Receive(macChipAdr, ee_buf, macChipEUI64len);
                i2cSubFunction = SUB_FKN_READ;
                break;
            default:
                mac_info.status = RIIC_ERR_TMO;
                i2cStopTimer();
        }
    }
}

void i2c_app::i2cStopTimer() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->stopAndReset(ProgramConfig::i2cTaskID);
}

void i2c_app::i2cStartInterrupt() {
    R_Config_RIIC0_Start();
}

void i2c_app::i2cTriggerTimer() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->start(ProgramConfig::i2cTaskID);
}


// read parameter from input line and prepare buffer for read or write from/to i2c bus
// format of input line, each parameter is separated by a space character:
// 1: <i2c chip address>                            chip address of the device in hex
// 2: <number of bytes to read or write>            decimal number expected
// 3: <number of additional parameters>             decimal number expected
// 4.. : <additional parameter 1> ..                parameter bytes in hex
int16_t i2c_app::terminalInput2Buffer(const char *text) {
    int cnt, rwAmount, chipAdr;
    text = charOperations::skip_token(text, ' ');
    if (*text == 0) {
        return -1;
    }

    // read address of the preferred i2c-Chip
    cnt = sscanf(text, "%x", &chipAdr);
    if (cnt != 1 || chipAdr > 0x7F) {
        return -1;
    }
    i2cAdr[0] = (uint8_t) chipAdr;
    text = charOperations::skip_token(text, ' ');
    if (*text == 0) {
        return -1;
    }

    // read number of bytes to read or write from device
    cnt = sscanf(text, "%u", &rwAmount);
    if (cnt != 1 || rwAmount > static_cast<int>(sizeof(ee_buf))) {
        return -1;
    }
    text = charOperations::skip_token(text, ' ');
    if (*text == 0) {  // exit if no parameters are needed
        return (int16_t) rwAmount;
    }

    // read number of additional parameters
    unsigned int paramAmount;
    cnt = sscanf(text, "%u", &paramAmount);
    if (cnt != 1 || paramAmount > static_cast<int>(sizeof(ee_buf))) {
        return -1;
    }

    // read additional parameters and transfer them to buffer
    unsigned int paramByte;
    unsigned int i, idx = 0;
    for (i=1; i <= paramAmount; i++) {
        text = charOperations::skip_token(text, ' ');
        if (*text == 0) {
            return -1;
        }
        cnt = sscanf(text, "%x", &paramByte);
        if (cnt != 1 || paramByte > 0xFF || idx > static_cast<int>(sizeof(ee_buf))) {
            return -1;
        }
        ee_buf[idx++] = (uint8_t) paramByte;
    }

    return (int16_t) rwAmount;
}

void i2c_app::i2cDumpError(MD_STATUS status) {
    if (i2cFunction == FKN_MAC_READ) {
        mac_info.status = RIIC_ERR_OTHER;
    }

    switch (status) {
        case MD_ERROR1:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "i2c arbitration lost");
            break;
        case MD_ERROR2:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "ic2 timeout");
            break;
        case MD_ERROR3:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "i2c NACK received");
            break;
        case MD_ERROR4:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "i2c communication sequence error");
            break;
    }
    termFilterPtr_->println(Global2::OutBuff);
}

void i2c_app::i2cWriteEnd(__attribute__((unused)) MD_STATUS dummy) {
    if (i2cFunction == FKN_MAC_READ) {
        // trigger read function
    }

    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "i2c write complete");
    termFilterPtr_->println(Global2::OutBuff);
}

void i2c_app::i2cReadEnd(__attribute__((unused)) MD_STATUS dummy) {
    if (i2cFunction == FKN_MAC_READ) {
        storeI2MacAdr();
        i2cFunction = FKN_IDLE;
    }

    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "i2c read complete");
    termFilterPtr_->println(Global2::OutBuff);
}

// Store Mac address from i2c read result
void i2c_app::storeI2MacAdr() {
    if ((i2c_read_result == RIIC_SUCCESS) && (i2c_write_result == RIIC_SUCCESS) &&
        (mac_info.status == RIIC_ERR_NO_INIT)) {
        mac_info.status = RIIC_SUCCESS;
        // get Mac address from buffer
        // check first if already 64 bits are read
        // TODO(AME):
        // Check if read result in ee_buf[0), ee_buf[1) for
        // chip 24AA02E48 is { 0xFF, 0xFF }
        // Background:
        // Both chips 24AA02E48, 24AA02E64 have the same i2c address 0x50
        // so actually read result has to be checked
        if ((ee_buf[0] == 0xFF) && (ee_buf[1] == 0xFF)) {
            // 48 bit available
            // copy read info (EUI-48) and generate an EUI-64
            mac_info.macAdr[0] = ee_buf[2];
            mac_info.macAdr[1] = ee_buf[3];
            mac_info.macAdr[2] = ee_buf[4];
            mac_info.macAdr[3] = 0xFF;
            mac_info.macAdr[4] = 0xFE;
            mac_info.macAdr[5] = ee_buf[5];
            mac_info.macAdr[6] = ee_buf[6];
            mac_info.macAdr[7] = ee_buf[7];
        } else {
            // 64 bit available
            memcpy(mac_info.macAdr, ee_buf, sizeof(mac_info.macAdr));
        }
    } else {
        memset(mac_info.macAdr, 0, sizeof(mac_info.macAdr));    // set result to 0, if no address is read
        mac_info.status = RIIC_ERR_OTHER;
    }
}

void i2c_app::getI2MacAdr(uint8_t arr[8]) {
    memcpy(arr, mac_info.macAdr, sizeof(mac_info.macAdr));
}

bool i2c_app::isMacAdrAvailable() {
    return mac_info.status == RIIC_SUCCESS;
}

void i2c_app::i2cStopInterrupt() {
    R_Config_RIIC0_Stop();
}



}  // namespace AME_SRC
