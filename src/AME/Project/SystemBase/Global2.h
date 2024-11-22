/*
 * Global2.h
 *
 *  Created on: 21.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_GLOBAL2_H_
#define SRC_AME_PROJECT_SYSTEMBASE_GLOBAL2_H_

#include "AutoConfigDefinitions.h"
#include "Driver/CompareTimer1.h"
// #define DEBUG
#define BOARD_ID 0      // used with COORDINATOR_ID = 0 => coordinator
#define COORDINATOR_ID 0
#define ALIVE_CHECK_WITH_ACK 0          // Flag according to ALIVE_CHECK_LEVEL=2:
                                        // >=1 the peers acknowledge the predecessor
// #ifdef RELEASE_CONFIGURATION
#define IS_FIRMWARE_DOWNLOAD_ENABLED 1  // determines whether the firmware is loaded to the G3 modem
                                        // after the system starts
                                        // 1=Firmware is loaded to the modem. 0=No Download
// #else
// #define IS_FIRMWARE_DOWNLOAD_ENABLED 0
// #endif

// TODO(AME): Collection of used hash tags for project software tasks:
// #MemoryOptimization
// #PossibleBug

namespace AME_SRC {

class Global2 {
 private:
    static CompareTimer1 cmt1;
 public:
    Global2();
    virtual ~Global2();
    static const char *pVersion;
    static const uint16_t outBuffMaxLength = 300;
    static const uint16_t inBuffMaxLength = 120;
    static char OutBuff[outBuffMaxLength];
    static char InBuff[inBuffMaxLength];
    static void set_version(const char*);
    static const char* get_version();
    static const char* get_version(char *dummy);

    static CompareTimer1* getCmt1() {
        return &cmt1;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_GLOBAL2_H_
