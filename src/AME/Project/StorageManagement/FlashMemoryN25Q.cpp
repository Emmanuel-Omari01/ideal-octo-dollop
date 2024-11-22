/*
 * FlashMemoryN25Q.cpp
 *
 *  Created on: July 23, 2024
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include <string.h>
#include "FlashMemoryN25Q.h"
#include "../SystemBase/Global2.h"
#include "../SystemBase/TimeMeasurement.h"

namespace AME_SRC {

TermContextFilter *FlashMemoryN25Q::termFilterPtr_ = NULL;
// rspi_evt_t FlashMemoryN25Q::callback_event;  // Storage for callback result data.
bool FlashMemoryN25Q::transferBusy = false;
flash_spi_mem_info_t FlashMemoryN25Q::flashMemInfo = {0, 0};


FlashMemoryN25Q::FlashMemoryN25Q(SerialDriver *termPtr): rspi_err(RSPI_SUCCESS) {
    termFilterPtr_ = new TermContextFilter(termPtr);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);

    rspi_handle = new rspi_config_block_s;

    rspi_command.cpha          = RSPI_SPCMD_CPHA_SAMPLE_ODD;
    rspi_command.cpol          = RSPI_SPCMD_CPOL_IDLE_LO;
    rspi_command.br_div        = RSPI_SPCMD_BR_DIV_1;
    rspi_command.ssl_assert    = RSPI_SPCMD_ASSERT_SSL0;
    rspi_command.ssl_negate    = RSPI_SPCMD_SSL_KEEP;
    rspi_command.bit_length    = RSPI_SPCMD_BIT_LENGTH_8;
    rspi_command.bit_order     = RSPI_SPCMD_ORDER_MSB_FIRST;
    rspi_command.next_delay    = RSPI_SPCMD_NEXT_DLY_SSLND;
    rspi_command.ssl_neg_delay = RSPI_SPCMD_SSL_NEG_DLY_SSLND;
    rspi_command.clock_delay   = RSPI_SPCMD_CLK_DLY_SPCKD;
    rspi_command.dummy         = RSPI_SPCMD_DUMMY;

    // Configure the RSPI settings structure
    rspi_setting.bps_target = 1000000;                     // Ask for 1Mbps bit-rate.
    rspi_setting.master_slave_mode = RSPI_MS_MODE_MASTER;  // Configure the RSPI as SPI Master.
#if RSPI_CFG_USE_GPIO_SSL == (0)
    rspi_setting.gpio_ssl = RSPI_IF_MODE_4WIRE;           // Set interface mode to 4-wire.
#else
    rspi_setting.gpio_ssl = RSPI_IF_MODE_3WIRE;           // Set interface mode to 3-wire.
#endif  // RSPI_CFG_USE_GPIO_SSL == (0)

    // Reneses serial Flash Driver
    flashOpened = false;
}

FlashMemoryN25Q::~FlashMemoryN25Q() {
    // TODO(AME): Auto-generated destructor stub
}


void FlashMemoryN25Q::getFlashID() {
    enum { kIDBlockSize = 18, flashIDCommand = 0x9E };
    memset(rxBuffer, 0, kIDBlockSize);
    memset(txBuffer, 0, kIDBlockSize);
    txBuffer[0] = flashIDCommand;

    // Open the RSPI channel using the API function
    rspi_err = R_RSPI_Open(0,                // RSPI channel number
                           &rspi_setting,    // Address of the RSPI settings structure.
                           rspi_command,
                           &rspi_callback,   // Address of user-defined callback function.
                           &rspi_handle);    // Address of where the handle is to be stored

    // If there were an error this would demonstrate error detection of API calls.
    if (RSPI_SUCCESS != rspi_err) {
        return;
    }

    /* Initialize I/O port pins for use with the RSPI peripheral.
     * This is specific to the MCU and ports chosen. */
    rspi_init_ports();

    transferBusy = true;
    rspi_err = R_RSPI_WriteRead(rspi_handle, rspi_command,
                               txBuffer, rxBuffer, kIDBlockSize);

    if (RSPI_SUCCESS == rspi_err) {  // Check for successful function return.
        waitForTransmissionComplete();
        deviceInfoToString(Global2::OutBuff, Global2::outBuffMaxLength);
        termFilterPtr_->println(Global2::OutBuff);
    }

    R_RSPI_Close(rspi_handle);
    unsetSPIportPins();
}


bool FlashMemoryN25Q::waitForTransmissionComplete() {
    bool timeout = false;
    TimeMeasurement::measureJob& timer = TimeMeasurement::registerTimedJob(-1);
    TimeMeasurement::restart();
    while (transferBusy) {
        uint32_t dt = TimeMeasurement::getJobDeltat(timer);
        if (dt >= 2) {  // 2ms
            timeout = true;
            break;
        }
    }

    TimeMeasurement::unregisterTimedJob(timer);
    return timeout;
}

// parameter timeoutMsInOut:
// Input:  allowed timeout time [ms]
// Output: waited=needed time [ms]
flash_spi_status_t FlashMemoryN25Q::checkAndWaitIfBusy(uint8_t deviceNo, flash_spi_poll_mode_t mode,
                                                    uint32_t *timeoutMsInOut) {
    flash_spi_status_t Ret;
    TimeMeasurement::measureJob& timer = TimeMeasurement::registerTimedJob(-1);
    TimeMeasurement::restart();
    uint32_t dt = 0;

    while (1) {
        /* FLASH is busy.
           User application can perform other processing while flash is busy. */

        Ret = R_FLASH_SPI_Polling(deviceNo, mode);
        if (FLASH_SPI_SUCCESS_BUSY != Ret) {
            /* FLASH is ready or error. */
            *timeoutMsInOut = dt;
            break;
        }
        dt = TimeMeasurement::getJobDeltat(timer);
        if (dt > *timeoutMsInOut) {  // timeout?
            Ret = FLASH_SPI_ERR_TIMEOUT;
            break;
        }
        // feed watchdog
        if ((dt & 0x3F) == 0) {    // every 64ms
            WatchDog::feed();
        }
    }

    return Ret;
}


bool FlashMemoryN25Q::isFlashBusy(uint8_t deviceNo, flash_spi_poll_mode_t mode) {
    flash_spi_status_t Ret = R_FLASH_SPI_Polling(deviceNo, mode);
    return Ret == FLASH_SPI_SUCCESS_BUSY;
}


char *FlashMemoryN25Q::deviceInfoToString(char *outBuf, uint32_t oBufSize) {
    // if (!transfer_busy) {
        uint32_t deviceInfo = (rxBuffer[1] << 16) | (rxBuffer[2] << 8) | rxBuffer[3];  // e.g. =>0x0020BA16
        char manufactor[20];
        if ((deviceInfo & MEM_TYPE_MASK) == MEM_TYPE_MICRON) {
            snprintf(manufactor, sizeof(manufactor), "Micron");
        } else if ((deviceInfo & MEM_TYPE_MASK) == MEM_TYPE_MACRONIX) {
            snprintf(manufactor, sizeof(manufactor), "Maxcronix");
        } else {
            snprintf(manufactor, sizeof(manufactor), "unknown SPI flash");
        }
        snprintf(outBuf, oBufSize, "Read ID: %x\r%s device detected.", deviceInfo, manufactor);
    // }
    return outBuf;
}

char* FlashMemoryN25Q::driverStatusToString(char *outBuf, uint32_t oBufSize,
                                        flash_spi_status_t status) {
    switch (status) {
    case FLASH_SPI_SUCCESS_BUSY:
        snprintf(outBuf, oBufSize, "Successful operation (Flash is busy)");
        break;
    case FLASH_SPI_SUCCESS:
        snprintf(outBuf, oBufSize, "Successful operation");
        break;
    case FLASH_SPI_ERR_PARAM:
        snprintf(outBuf, oBufSize, "Parameter error");
        break;
    case FLASH_SPI_ERR_HARD:
        snprintf(outBuf, oBufSize, "Hardware error");
        break;
    case FLASH_SPI_ERR_WP:
        snprintf(outBuf, oBufSize, "Write-protection error");
        break;
    case FLASH_SPI_ERR_TIMEOUT:
        snprintf(outBuf, oBufSize, "Time out error");
        break;
    case FLASH_SPI_ERR_OTHER:
        snprintf(outBuf, oBufSize, "Other error");
        break;
    case FLASH_SPI_ERR_WP_ADVANCED:
        snprintf(outBuf, oBufSize, "Advanced write-protection error");
        break;
    default:
        snprintf(outBuf, oBufSize, "Unknown status");
        break;
    }
    return outBuf;
}


/**********************************************************************************************************************
* Function Name: rspi_init_ports
* Description  : This function initializes the port pins associated with RSPI channel 0 for the RX... using
*                the macros defined in iodefine.h.
* Arguments    : None
* Return Value : None
**********************************************************************************************************************/
void FlashMemoryN25Q::rspi_init_ports() {
    /* Make the RSPI peripheral signal assignment selections in the MPC registers. */
    /* Set RSPI signal ports to peripheral mode. */
    R_RSPI_PinSet_RSPI0();

#if RSPI_CFG_USE_GPIO_SSL == (1)  // SPI 3-wire mode.
    /* Using GPIO to control the slave select signal (Synchronous 3-wire mode). */
    PORTA.PODR.BIT.B4 = 1;  // Set GPIO slave select pin high (slave de-selected).
    PORTA.PDR.BIT.B4  = 1;  // Set as output.
    PORTA.PMR.BIT.B4  = 0;  // Set as GPIO.
#endif
}

// short: drive all SPI pins as inputs
void FlashMemoryN25Q::unsetSPIportPins() {
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_MPC);

    /* Set RSPCKA pin */
    MPC.PC5PFS.BYTE = 0x00U;    // No special function
    PORTC.PMR.BIT.B5 = 0U;      // General I/O Pin
    PORTC.PDR.BIT.B5 = 0U;      // Input

    /* Set MOSIA pin */
    MPC.PC6PFS.BYTE = 0x00U;    // No special function
    PORTC.PMR.BIT.B6 = 0U;      // General I/O Pin
    PORTC.PDR.BIT.B6 = 0U;      // Input

    /* Set MISOA pin */
    MPC.PC7PFS.BYTE = 0x00U;    // No special function
    PORTC.PMR.BIT.B7 = 0U;      // General I/O Pin
    PORTC.PDR.BIT.B7 = 0U;      // Input

    /* Set SSLA0 pin */
    MPC.PC4PFS.BYTE = 0x00U;    // No special function
    PORTC.PMR.BIT.B4 = 0U;      // General I/O Pin
    PORTC.PDR.BIT.B4 = 0U;      // Input

    /* Set SSLA1 pin */
    // cpxResetPin on hardware version 1.0, 1.1
    MPC.PA0PFS.BYTE = 0x00U;    // No special function
    PORTA.PMR.BIT.B0 = 0U;      // General I/O Pin
    PORTA.PDR.BIT.B0 = 0U;      // Input

    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_MPC);
}


/**********************************************************************************************************************
* Function Name: rspi_callback
* Description  : This is an example of an RSPI callback function. Set signal that transfer is done.
* Arguments    : p_data - pointer to RSPI event code data.
* Return Value : None
**********************************************************************************************************************/
void FlashMemoryN25Q::rspi_callback(__attribute__((unused)) void *p_data) {
    // rspi_callback_data_t *pd = reinterpret_cast<rspi_callback_data_t *>(p_data);
    // callback_event = pd->event_code;
    transferBusy = false;
}

// Usage of Renesas serial Flash API ------------------------------------------------------

flash_spi_status_t FlashMemoryN25Q::openFlash(uint8_t devNo) {
    if (flashOpened) {
        return FLASH_SPI_SUCCESS;
    } else {
        flash_spi_status_t  Ret = R_FLASH_SPI_Open(devNo);
        flashOpened = Ret == FLASH_SPI_SUCCESS;
        return Ret;
    }
}


flash_spi_status_t FlashMemoryN25Q::getFlashMemoryInfo(uint8_t devNo) {
    flash_spi_status_t Ret = openFlash(devNo);

    if (Ret == FLASH_SPI_SUCCESS) {
        Ret = R_FLASH_SPI_GetMemoryInfo(devNo, &flashMemInfo);
    }
    return Ret;
}

char *FlashMemoryN25Q::flashMemoryInfoToString() {
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            "Memory size=%d bytes, page size=%d bytes",
            flashMemInfo.mem_size, flashMemInfo.wpag_size);
    return Global2::OutBuff;
}


flash_spi_status_t FlashMemoryN25Q::readFlash(uint8_t devNo, uint32_t address, uint32_t size) {
    flash_spi_info_t flashInfoRd;

    flashInfoRd.addr   = address;
    flashInfoRd.cnt    = size;
    flashInfoRd.p_data = &rxBuffer[0];
    flashInfoRd.op_mode = FLASH_SPI_SINGLE;

    return R_FLASH_SPI_Read_Data(devNo, &flashInfoRd);
}

flash_spi_status_t FlashMemoryN25Q::writeFlash(uint8_t devNo, uint32_t address, uint32_t size) {
    flash_spi_info_t flashInfoWr;

    flashInfoWr.addr   = address;
    flashInfoWr.cnt    = size;
    flashInfoWr.p_data = &txBuffer[0];
    flashInfoWr.op_mode = FLASH_SPI_SINGLE;

    return R_FLASH_SPI_Write_Data_Page(devNo, &flashInfoWr);
}


/***
 * Memory Organization. Source: Macronix MX25L3233F manual, Rev. 1.7, Feb. 19, 2021
 * Notice: the (mounted) Micron device N25Q032A13ESC40F has an address range of 0...7FFFFF (8 MByte)
 *         => the sector range is 0...2047.
 *         => the block  range is 0...127 (64k blocks)
 *         => the block  range is 0...255 (32k blocks)
  | Block(64k-byte) | Block(32K-byte) | Sector (4K-byte) | Address Range   |
  |-----------------+-----------------+------------------+-----------------
  |                 | 127             | 1023             | 3FF000h 3FFFFFh |
  |                 |                 | …                | …               |
  |                 |                 | 1016             | 3F8000h 3F8FFFh |
  |     63          +-----------------+------------------+-----------------
  |                 | 126             | 1015             | 3F7000h 3F7FFFh |
  |                 |                 | …                | …               |
  |                 |                 | 1008             | 3F0000h 3F0FFFh |
  |-----------------+-----------------+------------------+-----------------|
  |                 | 125             | 1007             | 3EF000h 3EFFFF  |
  |                 |                 | …                | …               |
  |                 |                 | 1000             | 3E8000h 3E8FFFh |
  |     62          +-----------------+------------------+-----------------|
  |                 | 124             | 999              | 3E7000h 3E7FFFh |
  |                 |                 | …                | …               |
  |                 |                 | 992              | 3E0000h 3E0FFFh |
  |-----------------+-----------------+------------------+-----------------|
  |                 | 123             | 991              | 3DF000h 3DFFFFh |
  |                 |                 | …                | …               |
  |                 |                 | 984              | 3D8000h 3D8FFFh |
  |     61          +-----------------+------------------+-----------------|
  |                 | 122             | 983              | 3D7000h 3D7FFFh |
  |                 |                 | …                | …               |
  |                 |                 | 976              | 3D0000h 3D0FFFh |
  |-----------------+-----------------+------------------+-----------------|

                                …

  |-----------------+-----------------+------------------+-----------------|
  |                 | 5               | 47               | 02F000h 02FFFFh |
  |                 |                 | …                | …               |
  |                 |                 | 40               | 02F000h 02FFFFh |
  |     2           +-----------------+------------------+-----------------
  |                 | 4               | 39               | 027000h 027FFFh |
  |                 |                 | …                | …               |
  |                 |                 | 32               | 020000h 020FFFh |
  |-----------------+-----------------+------------------+-----------------|
  |                 | 3               | 31               | 01F000h 01FFFFh |
  |                 |                 | …                | …               |
  |                 |                 | 24               | 018000h 018FFFh |
  |     1           +-----------------+------------------+-----------------|
  |                 | 2               | 23               | 017000h 017FFFh |
  |                 |                 | …                | …               |
  |                 |                 | 16               | 010000h 010FFFh |
  |-----------------+-----------------+------------------+-----------------|
  |                 | 1               | 15               | 00F000h 00FFFFh |
  |                 |                 | …                | …               |
  |                 |                 | 8                | 008000h 008FFFh |
  |     0           +-----------------+------------------+-----------------|
  |                 | 0               | 7                | 007000h 007FFFh |
  |                 |                 | …                | …               |
  |                 |                 | 0                | 000000h 000FFFh |
  |-----------------+-----------------+------------------+-----------------|
***/

// Erase flash device with the selected mode.
// Notice:
// With mode FLASH_SPI_MODE_C_ERASE FLASH_SPI_SUCCESS is reported though the chip is not erased.
flash_spi_status_t FlashMemoryN25Q::eraseFlash(uint8_t  devNo, uint32_t address,
                              flash_spi_erase_mode_t mode, bool waitComplete) {
    flash_spi_erase_info_t Flash_Info_E;
    flash_spi_status_t ret;

    ret = clearWriteProtection(devNo);
    if (ret != FLASH_SPI_SUCCESS) {
        return ret;
    }

    Flash_Info_E.addr   = address;
    Flash_Info_E.mode   = mode;  // FLASH_SPI_MODE_C_ERASE    => chip erase
                                 // FLASH_SPI_MODE_S_ERASE    => sector erase
                                 // FLASH_SPI_MODE_B32K_ERASE => 32k block erase
                                 // FLASH_SPI_MODE_B64K_ERASE => 64k block erase
    ret = R_FLASH_SPI_Erase(devNo, &Flash_Info_E);
    if (FLASH_SPI_SUCCESS > ret) {
        return ret;
    }

    if (waitComplete) {
        uint32_t timeout;   // [ms]
        switch (mode) {
        case FLASH_SPI_MODE_S_ERASE:    timeout = FLASH_SE_BUSY_WAIT; break;
        case FLASH_SPI_MODE_B32K_ERASE: timeout = FLASH_BE32K_BUSY_WAIT; break;
        case FLASH_SPI_MODE_B64K_ERASE: timeout = FLASH_BE64K_BUSY_WAIT; break;
        case FLASH_SPI_MODE_C_ERASE:
        default:
            timeout = FLASH_CE_BUSY_WAIT;
            break;
        }
        ret = checkAndWaitIfBusy(devNo, FLASH_SPI_MODE_ERASE_POLL, &timeout);
        if (ret != FLASH_SPI_SUCCESS) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                     "Timeout after %dms", timeout);
            termFilterPtr_->println(Global2::OutBuff);
        }
    }
    return ret;
}


// Disable write or erase access
flash_spi_status_t FlashMemoryN25Q::setFlashWriteDisable(uint8_t devNo) {
    return R_FLASH_SPI_Write_Di(devNo);
}


flash_spi_status_t FlashMemoryN25Q::clearWriteProtection(uint8_t devno) {
    flash_spi_status_t ret = FLASH_SPI_SUCCESS;
    uint8_t     gStat;
    uint32_t    timeout = FLASH_WR_BUSY_WAIT;

    ret = R_FLASH_SPI_Set_Write_Protect(devno, 0);     // ToDo (AME): check 2nd parameter
    if (FLASH_SPI_SUCCESS > ret) {
        return ret;
    }

    ret = checkAndWaitIfBusy(devno, FLASH_SPI_MODE_REG_WRITE_POLL, &timeout);
    if (FLASH_SPI_SUCCESS != ret) {
        return ret;
    }

    ret = R_FLASH_SPI_Read_Status(devno, &gStat);
    if (FLASH_SPI_SUCCESS > ret) {
        return ret;
    }

    if (gStat & (0x0f << 2) != 0x00) {
        return FLASH_SPI_ERR_OTHER;
    }
    return ret;
}

// - - - - - - - - - - - - - - - - - - - - -
// IStorage interface

// ToDo(AME):
// - implement flash write function here.
// - Separate in several block write parts if needed
void FlashMemoryN25Q::store(const uint8_t *data, uint32_t address, uint16_t size) {
}


// ToDo(AME): erase several blocks here
void FlashMemoryN25Q::clear(uint32_t address, uint16_t size) {
}


// ToDo(AME): check if sub-functions have to be placed here
void FlashMemoryN25Q::setup() {
}

// ToDo(AME): implement function with status of store, clear access
bool FlashMemoryN25Q::isReady() {
    return true;
}

// ToDo(AME): implement function with further status messages
IStorage::storeStates FlashMemoryN25Q::getStatus() {
    return kStoreSuccess;
}


} /* namespace AME_SRC */
