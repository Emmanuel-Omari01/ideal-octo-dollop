/*
 * MemoryInformation.cpp
 *
 *  Created on: Nov 10, 2023
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "MemoryInformation.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../../../Renesas/Generated/r_config/r_bsp_config.h"
#include "../SystemLogic/I18N.h"

extern void *__malloc_sbrk_base;

namespace AME_SRC {
// extern char _text, etext;    // ROM

MemoryInformation::MemoryInformation() = default;

MemoryInformation::~MemoryInformation() = default;

#pragma GCC diagnostic ignored "-Wstack-usage="
char* MemoryInformation::toString(char *buffer, uint32_t bufLen, uint32_t select) {
    const unsigned int rom_max = 524288;
    const unsigned int ram_max = 65536;

    const unsigned int ustack_size = BSP_CFG_USTACK_BYTES;
    const unsigned int istack_size = BSP_CFG_ISTACK_BYTES;
    char line[80];
    unsigned int start, ende, delta, pc, summe;
    void *pHeap;

    *buffer = 0;
    if (select == head) {
        snprintf(buffer, bufLen, TX::getText(TX::MemInfoTitle));
        delta = get_rom_size();
        pc = (delta * 100) / rom_max;
        snprintf(line, sizeof(line), TX::getText(TX::MemRomUsage), delta, pc);
        strncat(buffer, line, bufLen);
        return buffer;
    }

    delta = get_used_var_size();
    start = delta;                  // start of heap
    summe = delta;
    pc = (delta * 100) / ram_max;
    if (select == ram) {
        snprintf(line, sizeof(line), TX::getText(TX::MemRamUsage), delta, pc);
        strncat(buffer, line, bufLen);
    }

    start = (unsigned int) __malloc_sbrk_base;
    pHeap = malloc(10);
    if (pHeap) {        // pro form
        free(pHeap);
    }
    delta = ((unsigned int) pHeap) - start;
    summe+= delta;
    pc = (delta * 100) / ram_max;
    if (select == ram) {
        snprintf(line, sizeof(line), TX::getText(TX::MemDynVars), delta, pc);
        strncat(buffer, line, bufLen);
    }

    start = get_end_of_ustack();
    ende  = search_top_of_stack(reinterpret_cast<uint32_t*>(start));
    delta = start - ende;
    summe+= delta;
    pc = (delta * 100) / ustack_size;

    if (select == user_stack) {
        snprintf(line, sizeof(line), TX::getText(TX::MemUserStack), ustack_size, delta, pc);
        strncat(buffer, line, bufLen);
    }

    start = get_end_of_istack();
    ende  = search_top_of_stack(reinterpret_cast<uint32_t*>(start));
    delta = start - ende;
    summe+= delta;
    pc = (delta * 100) / istack_size;
    if (select == int_stack) {
        snprintf(line, sizeof(line), TX::getText(TX::MemIntStack), istack_size, delta, pc);
        strncat(buffer, line, bufLen);
    }

    pc = (summe * 100) / ram_max;
    if (select == summary) {
        snprintf(line, sizeof(line), TX::getText(TX::MemSummaryRAM), summe, pc);
        strncat(buffer, line, bufLen);
    }

    return buffer;
}

uint32_t MemoryInformation::search_top_of_stack(uint32_t *ptr) {
    const uint32_t pattern = 0x3F3F3F3F;  // ToDo(AME): obtain pattern from start.s
    const int32_t repeats = 10;
    int32_t max = 0x1000 / 4;
    int32_t i = 0;
    ptr = reinterpret_cast<uint32_t*>(((uint32_t) ptr & 0xFFFFFFFC));  // force uint32_t access

    while (i < repeats && max > 0) {
        i = 0;
        while (*ptr != pattern && max > 0) {
            ptr--;
            max--;
        }
        if (*ptr == pattern) {
            while (*ptr == pattern && max > 0 && i < repeats) {
                ptr--;
                max--;
                i++;
            }
        }
    }
    return (uint32_t) (ptr + repeats - 1);
}

}  // namespace AME_SRC

