/*
 * charOperations.cpp
 *
 *  Created on: 09.07.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "CharOperations.h"

#include <cstring>
#include <cstdlib>

namespace charOperations {

/***
 * This method reads the first argument of a terminal command and returns the number read.
 *  Example: "command 32" -> 32
 * @param command The command to be converted
 * @return Number that was read from the first argument
 */
unsigned char commandArgumentToNumber(char *command) {
    char *ptr, *savePtr;
    ptr = strtok_r(command, " ", &savePtr);
    ptr = strtok_r(nullptr, " ", &savePtr);
    return atoi(ptr);
}

// Read a word until the delimiter
const char* skip_token(const char *code, char token) {
    while (*code) {
        if (*code++ == token)
            break;
    }
    return code;
}

}  // namespace charOperations
