/*
 * Global.h
 *
 *     Created on: 07.01.2021
 * Last modified : 11.01.2021
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 *
 * Header file with access to c variable types
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_GLOBAL_H_
#define SRC_AME_PROJECT_SYSTEMBASE_GLOBAL_H_


#include <stdbool.h>
#include <stdint.h>

// #include "Command.h"
// namespace AME_SRC {


#define U8      unsigned char
#define U16     uint16_t            // unsigned short
#define U32     uint32_t            // unsigned long
#define I8      signed char
#define I16     i16_t               // signed short
#define I32     i32_t               // signed long
#ifndef NULL
    #define NULL 0
#endif


typedef union {
  struct { U16 lw;
           U16 hw;
         } word;
  struct { U8 byte0;
           U8 byte1;
           U8 byte2;
           U8 byte3;
          } byte;
  U8  bx[4];
  U32 all;
}var32;

// Terminal terminal;
// } // namespace

#endif  // SRC_AME_PROJECT_SYSTEMBASE_GLOBAL_H_
