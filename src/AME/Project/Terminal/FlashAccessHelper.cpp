/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include "FlashAccessHelper.h"
// User
#include "../SystemBase/ProgramConfig.h"
#include "../HelpStructures/CharOperations.h"
#include "../Terminal/TermContextFilter.h"

namespace AME_SRC {

FlashAccessHelper::FlashAccessHelper() {
    // There is no need to use the constructor.
    // Only static functions are implemented
}

FlashAccessHelper::~FlashAccessHelper() {
}


char* FlashAccessHelper::spiFlashTestInterface(char *text) {
    ProgramConfig::getTerminalFilter()->println(text);  // echo input line

    const char *txt = charOperations::skip_token(text, ' ');
    FlashMemoryN25Q *FlashMemoryN25Q_ = ProgramConfig::getFlashMemoryDriverPtr();
    flash_spi_status_t ret;

    if (strstr(txt, "getSpiID")) {
        FlashMemoryN25Q_->getFlashID();
        return const_cast<char *>("done if.");
    }

    if (strstr(txt, "getMemoryInfo")) {
        ret = FlashMemoryN25Q_->getFlashMemoryInfo(FLASH_SPI_DEV0);
        if (ret == FLASH_SPI_SUCCESS) {
            return FlashMemoryN25Q::flashMemoryInfoToString();
        }
        return FlashMemoryN25Q_->driverStatusToString(Global2::OutBuff,
                Global2::outBuffMaxLength, ret);
    }

    if (strstr(txt, "read")) {      // read <adr> <size>
        return readStream(txt);
    }

    if (strstr(txt, "write_text")) {    // write_text <adr> <text>
        return writeText(txt);
    }

    if (strstr(txt, "write_stream")) {  // write_stream <adr> <byte1> ... <byteN>
        return writeStream(txt);
    }

    if (strstr(txt, "erase_sector")) {  // erase_sector <sector>
        return eraseFlash(txt, FLASH_SPI_MODE_S_ERASE);
    }
    if (strstr(txt, "erase_32k")) {  // erase_32k <block>
        return eraseFlash(txt, FLASH_SPI_MODE_B32K_ERASE);
    }
    if (strstr(txt, "erase_64k")) {  // erase_64k <block>
        return eraseFlash(txt, FLASH_SPI_MODE_B64K_ERASE);
    }
    if (strstr(txt, "erase_chip")) {  // erase_chip
        return eraseFlash(txt, FLASH_SPI_MODE_C_ERASE);
    }

    if (strstr(txt, "isBusy")) {
        return isFlashBusy(txt);
    }

    return const_cast<char *>("nothing to do.");
}

char *FlashAccessHelper::readStream(const char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    FlashMemoryN25Q *FlashMemoryN25Q_ = ProgramConfig::getFlashMemoryDriverPtr();
    uint32_t val1, val2;

    int32_t cnt = sscanf(txt, "%x %u", &val1, &val2);
    if (cnt < 2) {
        return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
    }
    flash_spi_status_t ret = FlashMemoryN25Q_->readFlash(FLASH_SPI_DEV0, val1, val2);
    if (ret == FLASH_SPI_SUCCESS) {
        uint32_t adr = (uint32_t) FlashMemoryN25Q_->getRxBufferAdr();
        ProgramConfig::getTerminal()->Dump_Command(adr);
    }
    return FlashMemoryN25Q_->driverStatusToString(Global2::OutBuff,
            Global2::outBuffMaxLength, ret);
}

char *FlashAccessHelper::writeText(const char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    FlashMemoryN25Q *FlashMemoryN25Q_ = ProgramConfig::getFlashMemoryDriverPtr();
    uint32_t val1, val2;
    int32_t cnt;

    cnt = sscanf(txt, "%x", &val1);
    if (cnt < 1) {
        return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
    }
    // copy text to write buffer
    txt = charOperations::skip_token(txt, ' ');
    val2 = strlen(txt);
    if (val2 > FlashMemoryN25Q::kTXSize) {
        return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
    }
    memcpy(FlashMemoryN25Q_->getTxBufferAdr(), txt, val2);
    flash_spi_status_t ret = FlashMemoryN25Q_->writeFlash(FLASH_SPI_DEV0, val1, val2);
    return FlashMemoryN25Q_->driverStatusToString(Global2::OutBuff,
            Global2::outBuffMaxLength, ret);
}

char *FlashAccessHelper::writeStream(const char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    FlashMemoryN25Q *FlashMemoryN25Q_ = ProgramConfig::getFlashMemoryDriverPtr();
    uint32_t val1, val2;
    int32_t cnt;

    cnt = sscanf(txt, "%x", &val1);
    if (cnt < 1) {
        return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
    }
    // decode stream of bytes from input line and store it in write buffer
    uint32_t byte;
    val2 = 0;
    uint8_t *p = FlashMemoryN25Q_->getTxBufferAdr();
    uint32_t max = FlashMemoryN25Q::kTXSize;
    while (max > 0) {
        txt +=3;
        cnt = sscanf(txt, " %02X", &byte);
        if ((cnt == 1) && (byte < 256)) {
            *p++ = (uint8_t) byte;
            val2++;
            max--;
        } else {
            break;
        }
    }
    flash_spi_status_t ret = FlashMemoryN25Q_->writeFlash(FLASH_SPI_DEV0, val1, val2);
    return FlashMemoryN25Q_->driverStatusToString(Global2::OutBuff,
            Global2::outBuffMaxLength, ret);
}


char *FlashAccessHelper::isFlashBusy(const char *text) {
    FlashMemoryN25Q *FlashMemoryN25Q_ = ProgramConfig::getFlashMemoryDriverPtr();
    if (FlashMemoryN25Q_->isFlashBusy(FLASH_SPI_DEV0, FLASH_SPI_MODE_ERASE_POLL)) {
        return const_cast<char *>("busy");
    }
    return const_cast<char *>("not busy");
}


char *FlashAccessHelper::eraseFlash(const char *text, flash_spi_erase_mode_t mode) {
    const char *txt = charOperations::skip_token(text, ' ');
    FlashMemoryN25Q *FlashMemoryN25Q_ = ProgramConfig::getFlashMemoryDriverPtr();
    uint32_t val1 = 0;
    int32_t cnt;

    // Read ADDRESS(!) specifying the area to be deleted
    cnt = sscanf(txt, "%x", &val1);
    if ((cnt < 1) && (mode != FLASH_SPI_MODE_C_ERASE)) {
        return const_cast<char *>(TX::getText(TX::dTxtInvalidArguments));
    }

    flash_spi_status_t ret = FlashMemoryN25Q_->eraseFlash(FLASH_SPI_DEV0, val1, mode, true);
    return FlashMemoryN25Q_->driverStatusToString(Global2::OutBuff,
            Global2::outBuffMaxLength, ret);
}

} /* namespace AME_SRC */
