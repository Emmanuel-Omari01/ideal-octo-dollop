/*
 * SerialDriver.h
 *
 *  Created on: 11.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SERIALDRIVERS_SERIALDRIVER_H_
#define SRC_AME_PROJECT_TERMINAL_SERIALDRIVERS_SERIALDRIVER_H_

#include <cstdint>

#include "../Sci/SerialComInterface.h"

namespace AME_SRC {

class SerialDriver {
 public:
    explicit SerialDriver(SerialComInterface *newSciPtr);
    virtual ~SerialDriver();
    virtual void putchar(uint8_t ch);  // Stack serial output characters, global variant
    virtual char* get_buffer();        // Function for passing a receive buffer
    virtual void set_buffer(uint8_t count, char *array);  // Function for simulating a receive buffer

    void print(const char *str);  // Output 0-terminated string on the terminal interface
    void printFast(const char *str);
    void println(const char *str);  //      "       "       " with line feed
    void printFormat(const char *str);
    void printFormatLN(const char *str);
    void printIntLN(int);

    void printTillIndexWithLn(const char *message, int index);
    void printRaw(char *array, int length);
    void printRawFast(char *array, int length);
    int  txdFree();
    bool isTransmissionComplete();

    SerialComInterface* getSciPtr() const {
        return sciPtr_;
    }

    void setSciPtr(SerialComInterface *sciPtr) {
        sciPtr_ = sciPtr;
    }

 protected:
    SerialComInterface *sciPtr_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_SERIALDRIVERS_SERIALDRIVER_H_
