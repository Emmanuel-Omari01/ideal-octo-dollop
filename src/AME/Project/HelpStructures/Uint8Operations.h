/*
 * uint8Operations.h
 *
 *  Created on: 20.05.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_UINT8OPERATIONS_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_UINT8OPERATIONS_H_

#include <cstdint>

#ifdef uint8_t
// uint8_t is defined
#else
/***
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
***/
#endif

namespace uint8Operations {

uint32_t sum4BytesFromMSB(const uint8_t *bytes);
uint32_t sum4BytesFromLSB(const uint8_t *bytes);
uint16_t sum2BytesFromLSB(const uint8_t *bytes);

uint8_t sum2HexChars(const char *chars);

uint8_t* insertUint16intoUint8Array(uint8_t *bytes, uint16_t value);
uint8_t *insertUint32intoUint8Array(uint8_t *array, uint32_t value);

int searchUint8InArray(const uint8_t target, const uint8_t *array, const uint16_t len);

const uint8_t * inHex1(const uint8_t *inPtr, uint8_t *value, bool *lOk);
bool str2binArray(const uint8_t *inPtr, uint8_t *binArray, int8_t maxLen, int8_t *effLen);

// uint8_t hex2int(const uint8_t *hex);

}  // namespace uint8Operations

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_UINT8OPERATIONS_H_
