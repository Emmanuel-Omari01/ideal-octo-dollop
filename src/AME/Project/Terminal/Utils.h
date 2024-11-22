/*
 * Utils.h
 *
 *  Created on: 11.01.2021
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 * Last modified : 11.01.2021
 */

#ifndef SRC_AME_PROJECT_TERMINAL_UTILS_H_
#define SRC_AME_PROJECT_TERMINAL_UTILS_H_

#include <cstdint>

namespace AME_SRC {

class Utils {
 public:
    Utils();
    virtual ~Utils();       // Destructor, pro forma

    static char* writeHexDigit(char *out, int16_t value);
    static char* writeHex2(char *out, int16_t value);
    static char* writeHex4(char *out, int16_t value);
    static char* writeHex8(char *out, int32_t value);
    static char* writeBCDDigit(char *out, uint32_t value);
    static char* writeBCD2(char *out, uint32_t value);
    static char* inHex1(char *inPtr, uint8_t *value, bool *lOk);
    static char* inHex2(char *inPtr, uint8_t *value, bool *lOk);
    static char* inHex4(char *inPtr, uint16_t *value, bool *lOk);
    static char* inHexU32(char *inPtr, uint32_t *value, bool *lOk);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_UTILS_H_
