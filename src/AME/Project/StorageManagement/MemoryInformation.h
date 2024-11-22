/*
 * MemoryInformation.h
 *
 *  Created on: Nov 10, 2023
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_STORAGEMANAGEMENT_MEMORYINFORMATION_H_
#define SRC_AME_PROJECT_STORAGEMANAGEMENT_MEMORYINFORMATION_H_

#include <cstdint>

namespace AME_SRC {

class MemoryInformation {
 public:
    MemoryInformation();
    virtual ~MemoryInformation();

    enum eSelect { head = 1, ram, user_stack, int_stack, summary, info_entries = summary };
    static char* toString(char *buffer, uint32_t bufLen, uint32_t select);
 private:
    static uint32_t search_top_of_stack(uint32_t *ptr);
};
extern "C" unsigned int get_rom_size(void);
extern "C" unsigned int get_used_var_size(void);
extern "C" unsigned int get_end_of_ustack(void);
extern "C" unsigned int get_end_of_istack(void);

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_STORAGEMANAGEMENT_MEMORYINFORMATION_H_
