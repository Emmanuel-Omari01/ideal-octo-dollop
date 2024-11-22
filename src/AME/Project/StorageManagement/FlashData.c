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
*******************************************************************************
* Copyright (C) 2010 Renesas Electronics Corporation. All rights reserved.    */
/*******************************************************************************
* File Name    : flash_data.c
* Version      : 1.00
* Device       : R5F51118
* Tool Chain   : RX Family C Compiler
* H/W Platform : RX111
* Description  : Defines flash and ADC functions used in this sample.
*******************************************************************************/
/*******************************************************************************
* History	   : 23.08.2010 Ver. 1.00 First Release
*******************************************************************************/

/*******************************************************************************
* Project Includes
*******************************************************************************/
/* Provides standard string function definitions used in this file */

#include <stdio.h>
#include <string.h>
#include "../SystemBase/Global.h"
// Declares prototypes for functions defined in this file
#include <stdbool.h>
#include "FlashData.h"

#include <platform.h>
//#include "../../../Renesas/generated/r_flash_rx/src/targets/rx111/r_flash_rx111.h"

//
// DS: es gilt FLASH_TYPE_1
//

/*******************************************************************************
* Variables
*******************************************************************************/
// Imported global variable, list of block addresses array
const uint32_t g_flash_BlockAddresses[8] = { 
			FLASH_DF_BLOCK_0,
			FLASH_DF_BLOCK_1,
			FLASH_DF_BLOCK_2,
			FLASH_DF_BLOCK_3,
			FLASH_DF_BLOCK_4,
			FLASH_DF_BLOCK_5,
			FLASH_DF_BLOCK_6,
			FLASH_DF_BLOCK_7 }; // DB7

#if FLASH_DF_MIN_PGM_SIZE != (1)
U32 gFlashWriteAddr;			// Flash write address location global variable
#endif
// U32 gFlashWrErrCntAddr;		// Schreib-Adresse für den Fehlerzähler

#if FLASH_DF_MIN_PGM_SIZE == (1)
#ifndef EXCLUDE_FLASH_FCTN
U8 my_flash_sample[] = { 0x55, 0xAA };
#endif
#else
U8 gFlashWriteBuffer[48];	// Flash write buffer array
U8 my_flash_sample[] = "The quick brown fox jumps over the lazzy dog";
#endif
bool my_df_init = false;

/*******************************************************************************
* Local Function Prototypes
*******************************************************************************/
// initialisation function prototype declaration

//-----------------------------------------------------------------------------
// prüfe, ob Flash-Zugriffe initialisiert wurden
// Rückgabe: True: ja, alles klar
bool flash_is_init(void) 
{
	// if (!my_df_init)
	//	//se_println("Data-Flash Zugriffe noch nicht initialisiert!");
	return my_df_init;
}
/*******************************************************************************
* Outline		: Init_FlashData
* Description	: This function initialises the MCU flash area, allowing it to be
*				  read and written to by user code.
* Argument		: none
* Return value	: none
*******************************************************************************/
void Init_FlashData(void)
{
	// R_FlashCodeCopy();	// erforderlich wenn Flash-Rom aktualisiert werden soll

	// Enable MCU access to the data flash area
	// R_FlashDataAreaAccess(0xFFFF, 0xFFFF);
	flash_err_t err = R_FLASH_Open();
	if (err != FLASH_SUCCESS){
	//	//se_println("Flash Init failed");
	} else {
	//	//se_println("Data-Flash initialisiert");

		// Initialise, AME-Version
		// Set the initial flash write address to the start of block DB0
		#if FLASH_DF_MIN_PGM_SIZE != (1)
		gFlashWriteAddr = g_flash_BlockAddresses[0];
		#endif
		my_df_init = true;
	}

} // Init_FlashData
/*******************************************************************************
* Outline		: Erase_FlashData
* Description	: This function enters a for loop, and erases a block of data
*				  flash memory each iteration until all blocks have been erased.
* Argument		: user_adr: 0...7 Block-Nr des zu löschenden Blocks
*							0xFFFFFFFF: alle Blöcke
*							?			Adresse des zu löschenden Blocks
* Return value	: Rückmeldung von
*		  R_FlashErase 			FLASH_SUCCESS (0), FLASH_FAILURE (6), FLASH_BUSY (5) oder
*		  R_FlashDataAreaBlankCheck	FLASH_BLANK (0), FLASH_NOT_BLANK (1), FLASH_FAILURE (6), FLASH_BUSY (5), FLASH_ERROR_ADDRESS (3), FLASH_ERROR_BYTES(2)
*******************************************************************************/
flash_err_t Erase_FlashData(U32 user_block)
{
	flash_err_t err;	// Declare flash API error flag

	if (user_block<8) {
		err = R_FLASH_Erase((flash_block_address_t) g_flash_BlockAddresses[user_block], 1);	// , FLASH_NUM_BLOCKS_DF
	} else if (user_block == 0xFFFFFFFF) {
		err = R_FLASH_Erase(FLASH_DF_BLOCK_0, FLASH_NUM_BLOCKS_DF);
	} else 
		err = R_FLASH_Erase((flash_block_address_t) user_block, 1);

	return err;

} // Erase_FlashData
/*******************************************************************************
* Outline		: Write_FlashData
* Description	: This function writes the contents of gFlashWriteBuffer to the
*				  dataflash, at the location pointed by gFlashWriteAddr. If the
*				  number of bytes to write to flash is not a multiple of 8, the
*				  data is padded with null bytes (0x00) to make up to the
*				  nearest multiple of 8.
*				  Achtung: die Routine geht davon aus, dass in gFlashWriteBuffer
*				  ein Text (Null-terminiert) hinterlegt ist.
* Argument		: none
* Return value	: Anzahl der geschriebenen Bytes
* Sonstiges		: Test. Wird im Moment nicht genutzt
*******************************************************************************/
#if FLASH_DF_MIN_PGM_SIZE == (1)
#else
flash_err_t Write_FlashData(void)
{
	flash_err_t ret;			// Declare flash API error flag
	uint8_t pad_buffer[256];	// Declare data padding array and loop counter
	uint32_t * flash_ptr = (uint32_t *)gFlashWriteAddr;	// Declare pointer to flash write location

	// Declare the number of bytes variable, and initialise with the number of
	// bytes the variable gFlashWriteBuffer contains
	uint8_t num_bytes = sizeof(gFlashWriteBuffer);

	// Clear the contents of the flash write buffer array
	memset(pad_buffer, 0x00, 256);

	// Copy contents of the write buffer to the padding buffer
	strncpy((char*)pad_buffer, (char*)gFlashWriteBuffer, (size_t) num_bytes);

	// Check if number of bytes is greater than 256
	if(num_bytes > 256) {
		// Number of bytes to write too high, set error flag to 1
		return FLASH_ERR_BYTES;
	}
	// Check if number of bytes to write is a multiple of 8
	else if(num_bytes % 8u) {
		// Pad the data to write so it makes up to the nearest multiple of 8
		num_bytes += 8u - (num_bytes % 8u);
	}

	// Write contents of write buffer to data flash
					// src_address, dest_address, num_bytes
	ret = R_FLASH_Write((uint32_t)pad_buffer, gFlashWriteAddr, (uint32_t) num_bytes);
	if (ret !=FLASH_SUCCESS)
		return ret;

	// Compare memory locations to verify written data
	// DS: hier kommt es zu Vergleichsfehlern, wenn in gFlashWriteBuffer kein
	// Text hinterlegt ist
	if (memcmp(gFlashWriteBuffer, flash_ptr, (size_t) num_bytes))
		return FLASH_ERR_FAILURE;

	return FLASH_SUCCESS;

} // Write_FlashData
#endif
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
void ame_flash_test_write(char *dummy)
{
#if FLASH_DF_MIN_PGM_SIZE == (1)		// bei RX111 ist die min. Schreibgröße im Data-Flash 1 Byte (EEProm)
	uint32_t size;
	static uint32_t ame_adr = FLASH_DF_BLOCK_1;
#endif

	if (!flash_is_init())
		return;


#if FLASH_DF_MIN_PGM_SIZE == (1)
	size = FLASH_DF_MIN_PGM_SIZE;
	if (R_FLASH_Write((U32) my_flash_sample, ame_adr, (uint32_t) size) == FLASH_SUCCESS) {
		ame_adr += FLASH_DF_MIN_PGM_SIZE;
	}
#else
	// Clear the contents of the flash write buffer array
	memset(gFlashWriteBuffer, 0x00, sizeof(gFlashWriteBuffer));

	// Copy the contents of the sample buffer into the flash write buffer array
	// strncpy ((char*)gFlashWriteBuffer, (char*)adc_result, 8);
	memcpy( (char*)gFlashWriteBuffer, my_flash_sample, sizeof(my_flash_sample));

	// Write the contents of gFlashDataBuffer to flash memory, at the
	// location specified in the address variable gFlashWriteAddr
	if (Write_FlashData() == FLASH_SUCCESS) {
		gFlashWriteAddr += sizeof(my_flash_sample);
		// auf nächste durch 8 teilbare Adresse setzen
		if (gFlashWriteAddr & 0x7) {
			gFlashWriteAddr &=0xFFFFFFF8;
			gFlashWriteAddr +=8;
		}
	}
#endif

} // ame_flash_test_write
#endif
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
// Test: Hänger im Flash-Treiber beseitigen
void ame_flash_release(char *dummy)
{ 
	flash_err_t err = flash_pe_mode_exit();
	if (FLASH_SUCCESS != err)
		flash_reset();

	flash_release_state();		// unlock driver

	//term_supress_warning(dummy);
	//("Flash Release");

} // ame_flash_release
#endif
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
// Einen Flash-Prom-Bereich prüfen (User-Data-Flash), ob er sich beschreiben lässt (Blank-Check)
void ame_flash_check(char *text)
{ U32 cnt, addr, size;
  flash_err_t ret	 = FLASH_ERR_FAILURE;	// Declare flash API error flag
  flash_res_t result = FLASH_RES_NOT_BLANK;
  bool lAdr_Ok = false;

	if (!flash_is_init())
		return;

  cnt = sscanf(text, "%x, %x", &addr, &size);
  if (cnt >= 1) {
	lAdr_Ok = (bool) ((addr>=FLASH_DF_BLOCK_0) && (addr<FLASH_DF_BLOCK_7 + FLASH_DF_BLOCK_SIZE));	// Adressbereich vom Daten-Flash
  }
  if ((cnt == 1) && lAdr_Ok)
	ret = R_FLASH_BlankCheck(addr, FLASH_DF_MIN_PGM_SIZE, &result);
  else if ((cnt==2) && lAdr_Ok && (size==8))
	ret = R_FLASH_BlankCheck(addr, size, &result);
  else
	//se_println("?");
  switch (ret) {
	case FLASH_SUCCESS:
//		if (result == FLASH_RES_NOT_BLANK)
//								//se_println("Flash not blank");
//							else if (result == FLASH_RES_BLANK)
//								//se_println("Flash is blank");
//							else
//								//se_println("Flash illegaler Rueckgabewert");
							break;
	case FLASH_ERR_FAILURE:	//se_println("Flash operation failed"); break;
	case FLASH_ERR_BUSY:	//se_println("Flash busy"); break;
	case FLASH_ERR_ADDRESS:	//se_println("Flash error address"); break;
	case FLASH_ERR_BYTES:	//se_println("Flash incorrect num. of bytes"); break;
							break;
  }

} // ame_flash_check
#endif
//------------------------------------------------------------------------------
// Wrapper zu R_FLASH_BlankCheck. 
// Wird benötigt, da ansonsten Linker-Fehler unresolved symbol zu R_FLASH_BlankCheck aufläuft?!
flash_err_t ame_check_flash_blank(uint32_t addr, uint32_t size, flash_res_t *result) {
	return R_FLASH_BlankCheck(addr, size, result);
}
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
U32 ame_flash_getNextRecord(U32 flsh_addr, U16 size)
{ U32 next_adr;
  U32 DF_MASK = 0xFFFFFFFF ^ (FLASH_DF_BLOCK_SIZE-1);

	next_adr = flsh_addr + 2*size;	// übernächste Adresse laut Benutzer-Record-Größe
	// Blockgrenze Flash-Prom überschritten?
	if ((flsh_addr & DF_MASK) == (next_adr & DF_MASK)) {
		// Nein, gleicher Block
		next_adr = flsh_addr + size;
		return next_adr;
	}
	return flsh_addr & DF_MASK;	// Anfang des Blocks zurückgeben

} // ame_flash_getNextRecord
#endif
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
// Suche im Data-Flash-Prom den letzten Record mit Benutzerdaten (beginnen mit key)
// Rückgabe: Adresse des Blocks, Ergebnis vom Blank-Check
U32 ame_flash_getLastRecord(U8 block_nr, U16 size, U16 key, flash_err_t *flash_state)
{ U32 flsh_addr = g_flash_BlockAddresses[block_nr];
  U32 next_adr;
  flash_err_t ret;			// Declare flash API error flag
  flash_res_t result;

  while (1) {
	// Zuerst prüfen, ob das Flash-Prom an der angegebenen Adresse nicht frei ist.
	// Hintergrund: wenn schon einmal im Flash-Prom mit dem angegebenen key
	// geschrieben wurde und anschliessend wieder gelöscht wurde, kann es sein
	// das der Key-Wert beim Lesen des Bereichs noch verfügbar ist, obwohl der
	// Bereich als gelöscht gemeldet wird.
	// Beim Anzeigen des Bereichs im Debugger "schwimmen" dann die Byte-Werte,
	// können aber durchaus noch mal den alten Wert liefern.
	ret = R_FLASH_BlankCheck(flsh_addr, 8, &result);			// Prüfung
	*flash_state = ret;

	if ((ret == FLASH_SUCCESS) && (result == FLASH_RES_BLANK)) {
		*flash_state = FLASH_SUCCESS;
		return flsh_addr;
	}
	if (* ((U16 *)flsh_addr) == key) {
		next_adr = ame_flash_getNextRecord(flsh_addr, size);
		// Blockgrenze Flash-Prom überschritten?
		if (next_adr > flsh_addr) {
			// Nein, gleicher Flash-Block
			ret = R_FLASH_BlankCheck(next_adr, 8, &result);			// Prüfung
			if ((ret == FLASH_SUCCESS) && (result==FLASH_RES_BLANK)) {
				*flash_state = FLASH_ERR_FAILURE;
				return flsh_addr;
			}
			if (* ((U16 *)next_adr) == key) {
				flsh_addr = next_adr;
				continue;
			}
		}
	}
	return flsh_addr;
  }

} // ame_flash_getLastRecord
#endif
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
// lese Benutzer-Record aus dem User-Data-Flash
// block_nr: 0...15 Block-Nr. vom User-Data-Flash
// Rückgabe: True: Block erfolgreich gelesen
bool ame_flash_read_record(void *my_record, U8 block_nr, U16 size, U16 key)
{ flash_err_t flash_state;
  bool result;
  U16 *p_flsh = (U16 *)ame_flash_getLastRecord(block_nr, size, key, &flash_state);	// Zeiger auf den Record ermitteln

  memcpy(my_record, p_flsh, (size_t) size);	// Record holen
  result = *((U16 *) my_record) == key;
  return result;	//  (bool) (flash_state == FLASH_SUCCESS);

} // ame_flash_read_record
#endif
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
// Benutzer-Record in das User-Data-Flash schreiben
// Rückgabe: true: alles klar
bool ame_flash_write_record(void *my_record, U8 block_nr,U16 size, U16 key)
{ U16 *p_flsh;
  U8  eff_block_nr = block_nr;	// effektive Block-Nr
//U8  bytes_written;
  bool do_erase;
  int cmp_result;
  flash_err_t ret, flash_state;			// Declare flash API error flag
  flash_res_t result;
  U32 flsh_addr, next_addr;

	if (!flash_is_init())
		return false;

  flsh_addr = ame_flash_getLastRecord(eff_block_nr, size, key, &flash_state);	// Zeiger auf den Record ermitteln

  p_flsh = (U16 *) flsh_addr;
  if ((flash_state!= FLASH_SUCCESS) && (*p_flsh == key)) {	// stimmt der Key?
	next_addr = ame_flash_getNextRecord(flsh_addr, size);	// nächsten Block adressieren
	do_erase  = (bool) (next_addr<flsh_addr);	// Löschen, falls nächste Adresse vor letzter Adresse liegt
	flsh_addr = next_addr;						// Nächste Adresse verwenden
  } else {
	flsh_addr   = g_flash_BlockAddresses[eff_block_nr];	// beginne von vorn
	ret = R_FLASH_BlankCheck(flsh_addr, FLASH_DF_BLOCK_SIZE, &result);		// Prüfung
	flash_state = ret;	// R_FlashDataAreaBlankCheck( flsh_addr, BLANK_CHECK_ENTIRE_BLOCK );
	do_erase    = (bool) ((flash_state != FLASH_SUCCESS) || (result != FLASH_RES_BLANK));
  }
  if (do_erase) {
	flash_state = R_FLASH_Erase((flash_block_address_t) g_flash_BlockAddresses[eff_block_nr], 1);
	if (flash_state != FLASH_SUCCESS)
		return false;
	flsh_addr = g_flash_BlockAddresses[eff_block_nr];	// beginne von vorn
  }



	if (size % 8) {
		return false;
	}
	flash_state = R_FLASH_Write((U32) my_record, flsh_addr, (uint32_t) size);
	if (flash_state != FLASH_SUCCESS) {
		return false;
	}
	// Compare memory locations to verify written data
	cmp_result = memcmp((void *)flsh_addr, my_record, (size_t) size);
	return (bool) (cmp_result == 0);

} // ame_flash_write_record
#endif
//------------------------------------------------------------------------------
// Prozessdaten ins Flash-Prom schreiben
// Rückgabe: true: alles klar
/***
bool ame_flash_write_block(void *my_record, U8 block_nr,U16 size)
{ U8  eff_block_nr = block_nr;	// effektive Block-Nr
  int cmp_result;
  U32 flsh_addr;
  flash_err_t ret, flash_state;			// Declare flash API error flag
  flash_res_t result;

	if (!flash_is_init())
		return false;

  flsh_addr   = g_flash_BlockAddresses[eff_block_nr];	// beginne von vorn
  ret = R_FLASH_BlankCheck(flsh_addr, FLASH_DF_BLOCK_SIZE, &result);		// Prüfung
  flash_state = ret;	// R_FlashDataAreaBlankCheck( flsh_addr, BLANK_CHECK_ENTIRE_BLOCK );

  if ((flash_state != FLASH_SUCCESS) || (result != FLASH_RES_BLANK)) {
	// Löschen erforderlich
	flash_state = R_FLASH_Erase((flash_block_address_t) g_flash_BlockAddresses[eff_block_nr], 1);
	if (flash_state != FLASH_SUCCESS)
		return false;
	flsh_addr = g_flash_BlockAddresses[eff_block_nr];
  }

  sprintf(global_out_buf,"Flash Write: %x", flsh_addr);
  //se_println(global_out_buf);

  if (size % 8)
	return false;

  flash_state = R_FLASH_Write((U32) my_record, flsh_addr, (uint32_t) size);
  if (flash_state != FLASH_SUCCESS)
	return false;

  // Compare memory locations to verify written data
  cmp_result = memcmp((void *)flsh_addr, my_record, (size_t) size);
  return (bool) (cmp_result == 0);

} // ame_flash_write_block
***/
//------------------------------------------------------------------------------
#ifndef EXCLUDE_FLASH_FCTN
flash_err_t ame_flash_erase(char *text)
{ U32 cnt, addr;
  flash_err_t ret;
  U32 DF_MASK = 0xFFFFFFFF ^ (FLASH_DF_BLOCK_SIZE-1);

  if (!flash_is_init())
	return FLASH_ERR_FAILURE;

  // Erase the data flash memory
  cnt = sscanf(text, "%x", &addr);
  if (cnt == 1)
	ret = Erase_FlashData(addr & DF_MASK);
  else {
	ret = Erase_FlashData(0);
	// Reset the flash write address to the start of block DB0
	#if FLASH_DF_MIN_PGM_SIZE != (1)
	gFlashWriteAddr = g_flash_BlockAddresses[0];
	#endif
  }

//  switch (ret) {
//	case FLASH_SUCCESS: 	//se_println("Flash blank"); break;
//	case FLASH_ERR_FAILURE:	//se_println("Flash operation failed"); break;
//	case FLASH_ERR_BUSY:	//se_println("Flash busy"); break;
//	case FLASH_ERR_ADDRESS:	//se_println("Flash error address"); break;
//	case FLASH_ERR_BYTES:	//se_println("Flash incorrect num. of bytes"); break;
//	default:
//		//se_println("Unknown response");
//  }
	return ret;

} // ame_flash_erase
#endif
//------------------------------------------------------------------------------
// Fehlerzähler bei Programmstart ermitteln
/***
void ame_read_err_cnt(void)
{
  U32 i,j;
  flash_err_t ret;			// Declare flash API error flag
  flash_res_t result;

	gFlashWrErrCntAddr = g_flash_BlockAddresses[BLOCK_ERR_CNT];
	i=0;

	while (i<FLASH_DF_BLOCK_SIZE) {
		ret = R_FLASH_BlankCheck(gFlashWrErrCntAddr, 8, &result);			// Prüfung
		if (ret == FLASH_SUCCESS) {
			if (result == FLASH_RES_BLANK) {
				return;
			}
		} else
			return;		// Fehler beim Lesen des Flashs
		for (j=0;j<7;j++) {
			if (* ((U8 *)gFlashWrErrCntAddr) == 0xFF)
				return;

			gFlashWrErrCntAddr++;
		}
		i+=8;
	}

} // ame_read_err_cnt
***/
//------------------------------------------------------------------------------
