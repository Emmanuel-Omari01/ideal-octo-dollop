/*
 * Utils.cpp
 *
 *  Created on: 11.01.2021
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 * Features:
 * Global used functions
 */

#include <cstring>
#include <cstdio>
// #include <stdlib.h>

#include "Utils.h"
#include "../SystemBase/Global.h"

namespace AME_SRC {

Utils::Utils() = default;

// Destructor: is not needed
Utils::~Utils() = default;

//------------------------------------------------------------------------------
// Output value as hex digit
char* Utils::writeHexDigit(char *out, int16_t value) {
    char tmp;

    tmp = 0x30 | (value & 0x0f);

    if (tmp > '9')      // correction to handle hex
        tmp += 0x07;
    *out++ = tmp;
    *out = 0;           // terminate output

    return out;
}
//------------------------------------------------------------------------------
// Output value as 2 hex digits
char* Utils::writeHex2(char *out, int16_t value) {
    out = writeHexDigit(out, value >> 4);
    return writeHexDigit(out, value);
}
//------------------------------------------------------------------------------
// Output value as 4 hex digits
char* Utils::writeHex4(char *out, int16_t value) {
    out = writeHex2(out, value >> 8);
    return writeHex2(out, value);
}
//------------------------------------------------------------------------------
// // Output value as 8 hex digits
char* Utils::writeHex8(char *out, int32_t value) {
    var32 temp;
    temp.all = value;

    out = writeHex4(out, temp.word.hw);
    return writeHex4(out, temp.word.lw);
}
//------------------------------------------------------------------------------
// Output value as a decimal digit
char* Utils::writeBCDDigit(char *out, uint32_t value) {
    *out++ = 0x30 | (value % 10);
    return out;
}
//------------------------------------------------------------------------------
// Output value as 2 decimal digits
char* Utils::writeBCD2(char *out, uint32_t value) {
    out = writeBCDDigit(out, value / 10);
    return writeBCDDigit(out, value);
}
//------------------------------------------------------------------------------
// Read in a single hex character
char* Utils::inHex1(char *inPtr, uint8_t *value, bool *lOk) {
    char ch;

    *lOk = true;        // default: Ok
    ch = *inPtr++;
    *value &= 0xF0;     // clear LSN from result

    if ((ch >= '0') && (ch <= '9'))
        *value |= ch - '0';
    else if ((ch >= 'A') && (ch <= 'F'))
        *value |= ch - 0x37;
    else if ((ch >= 'a') && (ch <= 'f'))
        *value |= ch - 0x57;
    else
        *lOk = false;   // invalid character

    return inPtr;
}
//------------------------------------------------------------------------------
// Read in 2 hex characters
char* Utils::inHex2(char *inPtr, uint8_t *value, bool *lOk) {
    inPtr = inHex1(inPtr, value, lOk);      // read first
    if (*lOk) {
        *value <<= 4;       // shift LSN to MSN
        inPtr = inHex1(inPtr, value, lOk);  // read second
    } else {
        inPtr++;    // skip 1 character
    }
    return inPtr;
}
//------------------------------------------------------------------------------
// Read U32 Hex value - amount of characters in the line is indefinite
char* Utils::inHexU32(char *inPtr, uint32_t *value, bool *lOk) {
    bool lTempOk;
    int i, nTemp;
    char *inTemp;

    nTemp = 0;
    inTemp = inPtr;
    for (i = 0; i <= 7; i++) {
        inTemp = inHex1(inTemp, reinterpret_cast<uint8_t*>(&nTemp), &lTempOk);
        if (i == 0)     // a minimum of 1 character must be available
            *lOk = lTempOk;
        if (lTempOk) {
            inPtr = inTemp;
            *value = nTemp;
            nTemp <<= 4;        // shift LSN to MSN
        } else {
            break;
        }
    }

    return inPtr + 1;   // Return: pointer to first valid character or first character a valid hex character
}
//------------------------------------------------------------------------------

}  // namespace AME_SRC
