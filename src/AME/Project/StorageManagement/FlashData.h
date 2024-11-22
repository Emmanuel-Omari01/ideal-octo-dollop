/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*******************************************************************************/
/*******************************************************************************
* Copyright (C) 2010 Renesas Electronics Corporation. All rights reserved.    */
/*******************************************************************************
* File Name		: flash_data.h
* Version 		: 1.00
* Device 		: R5F51118
* Tool Chain 	: RX Family C Compiler
* H/W Platform	: RX111
* Description	: Provides declarations for functions defined in flash_data.c
*******************************************************************************/
/*******************************************************************************
* History 		: 27.08.2010 Ver. 1.00 First Release
*******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "../SystemBase/Global.h"
#include "../../../Renesas/NotGenerated/r_cg_macrodriver.h"
#include "../../../Renesas/Generated/r_flash_rx/r_flash_rx_if.h"

/*******************************************************************************
* Macro definitions
*******************************************************************************/
// Multiple inclusion prevention macro
#ifndef FLASH_DATA_H
#define FLASH_DATA_H
#ifdef __cplusplus
 extern "C" {
#endif

#define EXCLUDE_FLASH_FCTN	1	// wenn definiert, werden diverse Funktionen NICHT bereit gestellt
								// => Code-Einsparung
/*******************************************************************************
* Global Function Prototypes
*******************************************************************************/
void Init_FlashData(void);	// Initialise flash sample function prototype declaration
flash_err_t Erase_FlashData(U32 user_block);

#ifndef EXCLUDE_FLASH_FCTN
void ame_flash_test_write(char *dummy);
void ame_flash_release(char *dummy);
flash_err_t ame_flash_erase(char *text);
void ame_flash_check(char *text);
bool ame_flash_read_record(void *my_record, U8 block_nr, U16 size, U16 key);
bool ame_flash_write_record(void *my_record, U8 block_nr, U16 size, U16 key);
bool ame_flash_write_block(void *my_record, U8 block_nr,U16 size);
#endif
flash_err_t ame_check_flash_blank(uint32_t addr, uint32_t size, flash_res_t *result);

#ifdef __cplusplus
 }
#endif

// End of multiple inclusion prevention macro
#endif
