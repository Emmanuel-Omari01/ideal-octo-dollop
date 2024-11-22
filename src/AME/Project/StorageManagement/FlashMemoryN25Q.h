/*
 * FlashMemoryN25Q.h
 *
 *  Created on: July 23, 2024
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_STORAGEMANAGEMENT_FLASHMEMORYN25Q_H_
#define SRC_AME_PROJECT_STORAGEMANAGEMENT_FLASHMEMORYN25Q_H_

extern "C" {
#include "../../Renesas/Generated/r_rspi_rx/r_rspi_rx_if.h"  // The RSPI module API interface file.
#include "../../Renesas/Generated/r_pincfg/r_rspi_rx_pinset.h"
#include "../../Renesas/Generated/r_flash_spi/r_flash_spi_if.h"
#include "../../Renesas/Generated/r_config/r_flash_spi_config.h"

}
#include "IStorage.h"
#include "../Terminal/TermContextFilter.h"
#include "../SystemBase/WatchDog/WatchDog.h"

namespace AME_SRC {

class FlashMemoryN25Q : IStorage {
 public:
    explicit FlashMemoryN25Q(SerialDriver *termPtr);
    virtual ~FlashMemoryN25Q();

    // IStorage Interface
    void store(const uint8_t *data, uint32_t address, uint16_t size);
    void clear(uint32_t address, uint16_t size);
    void setup();
    bool isReady();
    IStorage::storeStates getStatus();

    static const uint16_t kRXSize = 512;
    static const uint16_t kTXSize = 512;

    void getFlashID();
    flash_spi_status_t getFlashMemoryInfo(uint8_t devNo);
    flash_spi_status_t readFlash(uint8_t devNo, uint32_t address, uint32_t size);
    flash_spi_status_t writeFlash(uint8_t devNo, uint32_t address, uint32_t size);
    flash_spi_status_t eraseFlash(uint8_t devNo, uint32_t address,
                            flash_spi_erase_mode_t mode, bool waitComplete);
    bool isFlashBusy(uint8_t deviceNo, flash_spi_poll_mode_t mode);
    flash_spi_status_t setFlashWriteDisable(uint8_t devNo);
    flash_spi_status_t clearWriteProtection(uint8_t devno);
    char* driverStatusToString(char *outBuf, uint32_t oBufSize, flash_spi_status_t status);

    uint8_t *getRxBufferAdr() {
        return &rxBuffer[0];
    }
    uint8_t *getTxBufferAdr() {
        return &txBuffer[0];
    }
    static char *flashMemoryInfoToString();

 private:
    void rspi_init_ports();
    void unsetSPIportPins();
    char *deviceInfoToString(char *outBuf, uint32_t oBufSize);
    bool waitForTransmissionComplete();
    flash_spi_status_t checkAndWaitIfBusy(uint8_t deviceNo, flash_spi_poll_mode_t mode,
                                          uint32_t *timeoutMsInOut);
    flash_spi_status_t openFlash(uint8_t devNo);
    static void rspi_callback(void *p_data);

    static TermContextFilter *termFilterPtr_;
//  static rspi_evt_t callback_event;  // Storage for callback result data.
    static bool transferBusy;

    rspi_command_word_t  rspi_command;
    rspi_err_t           rspi_err;
    rspi_handle_t        rspi_handle;
    rspi_chnl_settings_t rspi_setting;

    uint8_t rxBuffer[kRXSize];
    uint8_t txBuffer[kTXSize];

    const uint32_t MEM_TYPE_MASK     = 0xFFF000;
    const uint32_t MEM_TYPE_MICRON   = 0x20B000;
    const uint32_t MEM_TYPE_MACRONIX = 0xC22000;    // e.g. MX25L3233F

    // Renesas serial Flash Driver
    const uint32_t FLASH_SE_BUSY_WAIT    = 300;     // Sector Erase busy timeout 200*1ms = 0.3s
                                                    // Manufacture: 200ms, test: ~240ms
    const uint32_t FLASH_BE32K_BUSY_WAIT = 1000;    // Block Erase (32KB) busy timeout 1,000*1ms = 1s
    const uint32_t FLASH_BE64K_BUSY_WAIT = 2000;    // Block Erase (64KB) busy timeout 2,000*1ms = 2s
    const uint32_t FLASH_CE_BUSY_WAIT    = 600000;  // Chip Erase busy timeout 600,000*1ms = 600s
    const uint32_t FLASH_WR_BUSY_WAIT    = 40;      // Write Register time out 40*1ms = 40ms

    static flash_spi_mem_info_t flashMemInfo;
    bool        flashOpened;
};

} /* namespace AME_SRC */

#endif  // SRC_AME_PROJECT_STORAGEMANAGEMENT_FLASHMEMORYN25Q_H_
