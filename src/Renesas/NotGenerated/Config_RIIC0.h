/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Config_RIIC0.h
* Component Version: 1.12.0
* Device(s)        : R5F51118AxFL
* Description      : This file implements device driver for Config_RIIC0.
***********************************************************************************************************************/

#ifndef SRC_RENESAS_NOTGENERATED_CONFIG_RIIC0_H_
#define SRC_RENESAS_NOTGENERATED_CONFIG_RIIC0_H_

#ifdef __cplusplus
  extern "C" {
#endif

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_riic.h"
#include "r_cg_macrodriver.h"

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define _F1_IIC0_SCL_LOW_LEVEL_PERIOD                          (0xF1U) /* SCL clock low-level period setting */
#define _F2_IIC0_SCL_HIGH_LEVEL_PERIOD                         (0xF2U) /* SCL clock high-level period setting */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_Config_RIIC0_Create(void);
void R_Config_RIIC0_Create_UserInit(void);
void R_Config_RIIC0_Start(void);
void R_Config_RIIC0_Stop(void);
MD_STATUS R_Config_RIIC0_Master_Send(uint16_t adr, uint8_t * const tx_buf, uint16_t tx_num);
MD_STATUS R_Config_RIIC0_Master_Send_Without_Stop(uint16_t adr, uint8_t * const tx_buf, uint16_t tx_num);
MD_STATUS R_Config_RIIC0_Master_Receive(uint16_t adr, uint8_t * const rx_buf, uint16_t rx_num);
void R_Config_RIIC0_IIC_StartCondition(void);
void R_Config_RIIC0_IIC_StopCondition(void);
static void r_Config_RIIC0_callback_transmitend(void);
static void r_Config_RIIC0_callback_receiveend(void);
static void r_Config_RIIC0_callback_error(MD_STATUS status);
/* Start user code for function. Do not edit comment generated here */
void r_Config_RIIC0_transmit_interrupt(void);
void r_Config_RIIC0_transmitend_interrupt(void);
void r_Config_RIIC0_receive_interrupt(void);
void r_Config_RIIC0_error_interrupt(void);

// AME defined variable and functions
static void (*riic0_cb_error)(MD_STATUS status);
static void (*riic0_cb_wr_end)(MD_STATUS status);
static void (*riic0_cb_rd_end)(MD_STATUS status);

enum eCBType { error, readend, writeend};
typedef void (*rric0_cb_fn_ptr)(MD_STATUS status);
void R_Config_RIIC0_register_cb(enum eCBType type, rric0_cb_fn_ptr cb);

#ifdef __cplusplus
  }
#endif

/* End user code. Do not edit comment generated here */
#endif  // SRC_RENESAS_NOTGENERATED_CONFIG_RIIC0_H_
