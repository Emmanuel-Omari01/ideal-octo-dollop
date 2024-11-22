/*
 * uint8Operations.cpp
 *
 *  Created on: 20.05.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "Uint8Operations.h"

#include <cstdlib>

namespace uint8Operations {

uint32_t sum4BytesFromMSB(const uint8_t *bytes) {
    uint32_t sum = 0;
    for (int j = 0; j < 4; j++) {
        sum |= ((*bytes++) << (8 * j));
    }
    return sum;
}

uint32_t sum4BytesFromLSB(const uint8_t *bytes) {
    uint32_t sum = 0;
    for (int j = 3; j >= 0; j--) {
        sum |= ((*bytes++) << (8 * j));
    }
    return sum;
}

uint16_t sum2BytesFromLSB(const uint8_t *bytes) {
    uint16_t sum = bytes[1];
    sum += bytes[0] << 8;
    return sum;
}

uint8_t sum2HexChars(const char *chars) {
    char *ptr = nullptr;
    char tmp[2] = { chars[0], chars[1] };
    return (uint8_t) strtoul(tmp, &ptr, 16);
}

uint8_t* insertUint16intoUint8Array(uint8_t *array, uint16_t value) {
    array[0] = value >> 8;
    array[1] = value & 0x00FF;
    return array;
}

uint8_t* insertUint32intoUint8Array(uint8_t *array, uint32_t value) {
    array[0] = value >> 24;
    array[1] = (value >> 16) & 0x000000FF;
    array[2] = (value >> 8) & 0x000000FF;
    array[3] = (value) & 0x000000FF;
    return array;
}

int searchUint8InArray(const uint8_t target, const uint8_t *array,
        const uint16_t len) {
    uint16_t index = 0;
    while (array[index] != target) {
        index++;
        if (index > len) {
            return -1;
        }
    }
    return target;
}

// Read a single hex character from stream
// Input:  inPtr - Pointer to a string with characters from the set "0" ... "9", "A" ... "F".
//                 Upper/lower case is ignored
// Output: value - 1 Binary-Digit 0...F
//         lOk   - false = Error in interpretation
//               - true  = everything is fine
// Return        - Pointer to the next character in the input string
const uint8_t* inHex1(const uint8_t *inPtr, uint8_t *value, bool *lOk) {
    uint8_t ch;

    *lOk = true;        // default: Ok
    ch = *inPtr++;      // Read characters and set pointer to next character
    *value &= 0xF0;     // clear LSN

    if ((ch >= '0') && (ch <= '9')) {
        *value |= ch - '0';
    } else if ((ch >= 'A') && (ch <= 'F')) {
        *value |= ch - 0x37;
    } else if ((ch >= 'a') && (ch <= 'f')) {
        *value |= ch - 0x57;
    } else {
        *lOk = false;   // invalid characters
    }

    return inPtr;
}  // inHex1

// Function str2binArray.
// The function converts a string with "0" ... "9", "A" ... "F" into a binary array
// The number of characters in the string must be even: (insert a leading zero if necessary)
// The character string "AB" returns a binArray with the entry 0xAB in the first byte and a length of one byte
// Input:   inPtr   - Pointer to a string with characters from the set "0" ... "9", "A" ... "F".
//                    Upper/lower case is ignored
//          binArray- Pointer to the binary array being written
//                    The operator must ensure that there is enough space in the memory - look at maxLen.
//          maxLen  - Number of maximum bytes that the routine is allowed to write
//                    When the maximum length is reached, reading is interrupted
//                    - regardless of whether other characters are available
// Output:  effLen  - Number of bytes written
//          binArray- The binary array being written
// Return   lOk     - false = Error in interpretation
//                  - true  = everything is fine
bool str2binArray(const uint8_t *inPtr, uint8_t *binArray, int8_t maxLen,
        int8_t *effLen) {
    bool lOk = true;
    uint32_t nibbleLen = 1;     // Number of nibbles used
    while ((maxLen > 0) && (*inPtr) && lOk) {
        for (int i = 0; i < 2; i++) {   // read 2 characters as 2 hex nibbles
            if (*inPtr) {
                inPtr = inHex1(inPtr, binArray, &lOk);
            } else {
                lOk = false;    // odd number of characters in the string
            }
            if (lOk) {
                if (i == 0) {
                    *binArray <<= 4;    // shift LSN -> MSN
                }
                nibbleLen++;
            } else {
                break;
            }
        }
        binArray++;
        if (--maxLen <= 0) {
            break;
        }
    }
    *effLen = nibbleLen >> 1;
    return lOk;
}

}  // namespace uint8Operations

/**
 * hex2int
 * take a hex string and convert it to a 8bit number (max 2 hex digits)
 */
 /***
uint8_t hex2int(const uint8_t *hex) {
    uint8_t val = 0;
    bool lOk;
    for (int i = 0;i<2;i++) {
        hex = inHex1(hex, &val, &lOk);
    }
    return val;
}
***/
