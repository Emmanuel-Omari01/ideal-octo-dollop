/*
 * SerialDriver.cpp
 *
 *  Created on: 11.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "SerialDriver.h"

#include <cstdint>
#include <cstdio>

#include "../SerialProfiles/SerialStandardProfile.h"
#include "../../SystemBase/Global2.h"

namespace AME_SRC {

extern void popTXDfast();

SerialDriver::SerialDriver(SerialComInterface *newSciPtr) :
        sciPtr_(newSciPtr) {
}

SerialDriver::~SerialDriver() = default;

void SerialDriver::putchar(uint8_t ch) {
    sciPtr_->pushTXD(ch);       // push characters for output
    if (ch == SerialStandardProfile::RETURN)
        putchar(SerialStandardProfile::LINEFEED);
}

// Output 0-terminated string on the terminal interface
void SerialDriver::print(const char *str) {
    while (*str) {
        putchar((uint8_t) (*str));
        str++;
    }
}

void SerialDriver::printFast(const char *str) {
    print(str);
    sciPtr_->popTXD();
}

void SerialDriver::printRaw(char *array, int length) {
    for (int i = 0; i < length; i++) {
        sciPtr_->pushTXD(*array);
        array++;
    }
}

void SerialDriver::printRawFast(char *array, int length) {
    if (sciPtr_->isFastMode()) {
        SerialComInterface::sci5Ptr->fastModeContainerPtr = array;
        SerialComInterface::sci5Ptr->fastModeContainerLength = length;
        popTXDfast();
    } else {
        printRaw(array, length);
        sciPtr_->popTXD();
    }
}

char* SerialDriver::get_buffer() {
    return sciPtr_->getProfilePtr()->getBuffer();
}

void SerialDriver::set_buffer(uint8_t count, char *array) {
    sciPtr_->getProfilePtr()->setBuffer(count, array);
}

int SerialDriver::txdFree(void) {
    return sciPtr_->txdFree();
}

bool SerialDriver::isTransmissionComplete() {
    return sciPtr_->isTransmissionComplete();
}

//------------------------------------------------------------------------------
void SerialDriver::println(const char *str) {
    print(str);
    putchar(SerialStandardProfile::RETURN);
}

void SerialDriver::printFormat(const char *str) {
    while (*str) {
        if (*str == '\n') {
            putchar(SerialStandardProfile::RETURN);
        } else {
            putchar((uint8_t) (*str));
        }
        str++;
    }
}

void SerialDriver::printFormatLN(const char *str) {
    printFormat(str);
    putchar(SerialStandardProfile::RETURN);
}

void SerialDriver::printIntLN(int i) {
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%d", i);
    println(Global2::OutBuff);
}

//------------------------------------------------------------------------------

// void SerialDriver::printRaw( char *array, int length) {
//      for(int i=0;i<length;i++){
//          putchar((U8) (*array));
//          array++;
//      }
//}

void SerialDriver::printTillIndexWithLn(const char *message, int index) {
    for (int i = 0; i < index; i++) {
        if (*message == '\n') {
            putchar(SerialStandardProfile::RETURN);
        } else {
            putchar((uint8_t) (*message));
        }
        message++;
    }
    putchar(SerialStandardProfile::RETURN);
}

}  // namespace AME_SRC

