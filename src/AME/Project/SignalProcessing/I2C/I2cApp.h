/*
 * i2capp.h
 *
 *  Created on: May 14, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_I2C_I2CAPP_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_I2C_I2CAPP_H_

#include <cstdint>

#include "../../../../Renesas/NotGenerated/Config_RIIC0.h"
#include "../../../../Renesas/NotGenerated/r_cg_macrodriver.h"
#include "../../Terminal/TermContextFilter.h"

namespace AME_SRC {

#define macChipAdr       0x50       // i2c address for 24AA02E64
#define macChipOfsEUI64  0xF8       // offset of EUI-64
#define macChipEUI64len  8          // 64 bits for EUI-64

class i2c_app {
 public:
    explicit i2c_app(SerialDriver *term);
    virtual ~i2c_app();

    /***********************************************************************************************************************
     Typedef definitions
     **********************************************************************************************************************/
    /*----------------------------------------------------------------------------*/
    /*   Define return values and values of channel state flag.                   */
    /*----------------------------------------------------------------------------*/
    typedef uint8_t riic_ch_dev_status_t;
        #define RIIC_NO_INIT        ((riic_ch_dev_status_t)(0)) /* Uninitialized state */
        #define RIIC_IDLE           ((riic_ch_dev_status_t)(1)) /* Successful operation */
        #define RIIC_FINISH         ((riic_ch_dev_status_t)(2)) /* Already idle state */
        #define RIIC_NACK           ((riic_ch_dev_status_t)(3)) /* Already idle state */
        #define RIIC_COMMUNICATION  ((riic_ch_dev_status_t)(4)) /* Successful operation */
        #define RIIC_AL             ((riic_ch_dev_status_t)(5)) /* Arbitration lost */
        #define RIIC_TMO            ((riic_ch_dev_status_t)(6)) /* Time Out */
        #define RIIC_ERROR          ((riic_ch_dev_status_t)(7)) /* error */

    /*----------------------------------------------------------------------------*/
    /*   Define iic information structure type.                                   */
    /*----------------------------------------------------------------------------*/
    typedef uint8_t riic_return_t;
        #define RIIC_SUCCESS           ((riic_return_t)(0))  /* Successful operation */
        #define RIIC_ERR_LOCK_FUNC     ((riic_return_t)(1))  /* Lock has already been acquired by another task. */
        #define RIIC_ERR_INVALID_CHAN  ((riic_return_t)(2))  /* None existent channel number */
        #define RIIC_ERR_INVALID_ARG   ((riic_return_t)(3))  /* Parameter error */
        #define RIIC_ERR_NO_INIT       ((riic_return_t)(4))  /* Uninitialized state */
        #define RIIC_ERR_BUS_BUSY      ((riic_return_t)(5))  /* Channel is on communication. */
        #define RIIC_ERR_AL            ((riic_return_t)(6))  /* Arbitration lost error */
        #define RIIC_ERR_TMO           ((riic_return_t)(7))  /* Time Out error */
        #define RIIC_ERR_OTHER         ((riic_return_t)(8))  /* Other error */

    /*----------------------------------------------------------------------------*/
    /*   Define riic status structure type.                                       */
    /*----------------------------------------------------------------------------*/
    typedef union {
        uint32_t LONG;
        struct {
            uint32_t rsv :12; /* reserve */
            uint32_t AAS2 :1; /* Slave2 address detection flag */
            uint32_t AAS1 :1; /* Slave1 address detection flag */
            uint32_t AAS0 :1; /* Slave0 address detection flag */
            uint32_t GCA :1; /* Generalcall address detection flag */
            uint32_t DID :1; /* DeviceID address detection flag */
            uint32_t HOA :1; /* Host address detection flag */
            uint32_t MST :1; /* Master mode / Slave mode flag */
            uint32_t TMO :1; /* Time out flag */
            uint32_t AL :1; /* Arbitration lost detection flag */
            uint32_t SP :1; /* Stop condition detection flag */
            uint32_t ST :1; /* Start condition detection flag */
            uint32_t RBUF :1; /* Receive buffer status flag */
            uint32_t SBUF :1; /* Send buffer status flag */
            uint32_t SCLO :1; /* SCL pin output control status */
            uint32_t SDAO :1; /* SDA pin output control status */
            uint32_t SCLI :1; /* SCL pin level */
            uint32_t SDAI :1; /* SDA pin level */
            uint32_t NACK :1; /* NACK detection flag */
            uint32_t TRS :1; /* Send mode / Receive mode flag */
            uint32_t BSY :1; /* Bus status flag s*/
        } BIT;
    } riic_mcu_status_t;

    // Internal functions to be done
    typedef uint8_t i2cFkn_t;
        #define FKN_IDLE       ((i2cFkn_t) (0))  // idle state
        #define FKN_MAC_READ   ((i2cFkn_t) (1))  // read mac address

    // Sub-Function
    typedef uint8_t i2cSubFkn_t;
        #define SUB_FKN_IDLE ((i2cSubFkn_t) (0))   // idle state
        #define SUB_FKN_READ ((i2cSubFkn_t) (1))   // read function
        #define SUB_FKN_WRITE ((i2cSubFkn_t) (2))  // write function

    typedef struct {
        uint8_t macAdr[8];      // EUI-64
        riic_return_t status;   // status of read operation
    } mac_info_t;

    static char *i2c_sample_write(char *text);
    static char *i2c_sample_read(char *text);
    static char *i2c_init(char *text);
    static char *i2cMacRead(char *text);
    static void getI2MacAdr(uint8_t arr[8]);
    static bool isMacAdrAvailable();
    static void i2cStopInterrupt();
    static void i2cStartInterrupt();

 private:
    static TermContextFilter *termFilterPtr_;
    static riic_return_t i2c_write_result;
    static riic_return_t i2c_read_result;
    static i2cFkn_t i2cFunction;
    static i2cSubFkn_t i2cSubFunction;

    static uint8_t i2cAdr[1];
    static uint8_t access_addr[2];
    static uint8_t ee_buf[128];

    static mac_info_t mac_info;
    static void storeI2MacAdr();

    static int16_t terminalInput2Buffer(const char *text);
    static void i2cDumpError(MD_STATUS status);
    static void i2cWriteEnd(MD_STATUS dummy);
    static void i2cReadEnd(MD_STATUS dummy);
    static void i2cTimed50ms();
    static void i2cStopTimer();
    static void i2cTriggerTimer();
};

// enum eCBType { error, readend, writeend};
extern  "C" void R_Config_RIIC0_Start(void);
extern  "C" void  R_Config_RIIC0_Stop(void);
extern  "C" MD_STATUS R_Config_RIIC0_Master_Send(uint16_t adr, uint8_t * const tx_buf, uint16_t tx_num);
extern  "C" MD_STATUS R_Config_RIIC0_Master_Receive(uint16_t adr, uint8_t * const rx_buf, uint16_t rx_num);
extern  "C" void R_Config_RIIC0_register_cb(enum eCBType type, rric0_cb_fn_ptr cb);


}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_I2C_I2CAPP_H_
