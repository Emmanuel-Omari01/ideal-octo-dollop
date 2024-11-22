/*
 * r_cg_lock.c
 *
 *  Created on: Jun 26, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */
#include <platform.h>

void R_System_Unlock() {
    /* Enable writing to registers related to operating modes, LPC, CGC and software reset */
    SYSTEM.PRCR.WORD = 0xA50FU;

    /* Enable writing to MPC pin function control registers */
    MPC.PWPR.BIT.B0WI = 0U;
    MPC.PWPR.BIT.PFSWE = 1U;
}

void R_System_Lock() {
    /* Disable writing to MPC pin function control registers */
    MPC.PWPR.BIT.PFSWE = 0U;
    MPC.PWPR.BIT.B0WI = 1U;

    /* Enable protection */
    SYSTEM.PRCR.WORD = 0xA500U;
}
