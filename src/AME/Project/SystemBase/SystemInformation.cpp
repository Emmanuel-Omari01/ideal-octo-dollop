/*
 * SystemInformation.cpp
 *
 *  Created on: Feb 12, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "SystemInformation.h"

#include <cstdio>
#include <string.h>

#include <platform.h>
#include "../SystemLogic/I18N.h"
// #include "../../../Renesas/Generated/r_bsp/mcu/rx111/register_access/gnuc/iodefine.h"

namespace AME_SRC {

SystemInformation::SystemInformation()= default;

SystemInformation::~SystemInformation() = default;

// Determine cause of last rest - look at RX111, User Manual Hardware,
// Chapter 6.3.7 Determination of Reset Generation Source
char* SystemInformation::resetSourceToString(char *output, unsigned int maxlen) {
    if (SYSTEM.RSTSR2.BIT.SWRF) {
        snprintf(output, maxlen, TX::getText(TX::ShowLastResetReason), "Software Reset");
    } else if (SYSTEM.RSTSR0.BIT.LVD2RF) {
        snprintf(output, maxlen, TX::getText(TX::ShowLastResetReason), "Voltage monitoring 2 reset");
    } else if (SYSTEM.RSTSR0.BIT.LVD1RF) {
        snprintf(output, maxlen, TX::getText(TX::ShowLastResetReason), "Voltage monitoring 1 reset");
    } else if (SYSTEM.RSTSR2.BIT.IWDTRF) {
        snprintf(output, maxlen, TX::getText(TX::ShowLastResetReason), "Independent watchdog timer reset");
    } else if (SYSTEM.RSTSR0.BIT.PORF) {
        snprintf(output, maxlen, TX::getText(TX::ShowLastResetReason), "Power-on reset");
    } else {
        snprintf(output, maxlen, TX::getText(TX::ShowLastResetReason), "RES# pin reset");
    }

    return output;
}

}  // namespace AME_SRC
