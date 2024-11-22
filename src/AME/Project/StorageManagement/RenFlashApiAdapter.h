/*
 * renFlashApiAdapter.h
 *
 *  Created on: May 28, 2024
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_STORAGEMANAGEMENT_RENFLASHAPIADAPTER_H_
#define SRC_AME_PROJECT_STORAGEMANAGEMENT_RENFLASHAPIADAPTER_H_

#include "../../../Renesas/Generated/r_flash_rx/r_flash_rx_if.h"

namespace AME_SRC {

extern "C" flash_err_t R_FLASH_Open(void);
extern "C" flash_err_t R_FLASH_Close(void);
extern "C" flash_err_t R_FLASH_Write(uint32_t src_address, uint32_t dest_address, uint32_t num_bytes);
extern "C" flash_err_t R_FLASH_Erase(flash_block_address_t block_start_address, uint32_t num_blocks);
extern "C" flash_err_t R_FLASH_BlankCheck(uint32_t address, uint32_t num_bytes, flash_res_t *blank_check_result);
extern "C" flash_err_t R_FLASH_Control(flash_cmd_t cmd, void *pcfg);

}

#endif  // SRC_AME_PROJECT_STORAGEMANAGEMENT_RENFLASHAPIADAPTER_H_
