/*
 * FlashAccessHelper.h
 *
 *  Created on: Aug 2, 2024
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_FLASHACCESSHELPER_H_
#define SRC_AME_PROJECT_TERMINAL_FLASHACCESSHELPER_H_

#include <stdlib.h>
#include <stdio.h>
#include "../StorageManagement/FlashMemoryN25Q.h"

namespace AME_SRC {

class FlashAccessHelper {
 public:
    FlashAccessHelper();
    virtual ~FlashAccessHelper();

    static char* spiFlashTestInterface(char *text);

 private:
    static char *readStream(const char *text);
    static char* writeText(const char *text);
    static char* writeStream(const char *text);
    static char* eraseFlash(const char *text, flash_spi_erase_mode_t mode);
    static char* isFlashBusy(const char *text);
};

} /* namespace AME_SRC */

#endif /* SRC_AME_PROJECT_TERMINAL_FLASHACCESSHELPER_H_ */
