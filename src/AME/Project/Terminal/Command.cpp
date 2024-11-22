/*
 * Commando.cpp
 *
 *  Created on: 13.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "Command.h"

#include <cstdlib>

namespace AME_SRC {

Command::Command() :
        name((const char*) "Dummy"), call_adr(NULL), description(const_cast<char*>("")) {
}
Command::Command(const char *newName, char* (*newAddress)(char *code)) :
    name(newName), call_adr(newAddress), description(const_cast<char*>("")) {
}
Command::Command(const char *newName, const char *newDescription,
        char* (*newAddress)(char *code)) :
    name(newName), description(newDescription), call_adr(newAddress) {
}
Command::~Command() = default;

}  // namespace AME_SRC

