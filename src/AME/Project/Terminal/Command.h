/*
 * Commando.h
 *
 *  Created on: 13.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_COMMAND_H_
#define SRC_AME_PROJECT_TERMINAL_COMMAND_H_

namespace AME_SRC {

class Command {
 public:
    const char *name;
    const char *description;
    char* (*call_adr)(char *code);
    Command();
    Command(const char *newName, char* (*newAddress)(char *code));
    Command(const char *newName, const char *newDescription,
            char* (*newAddress)(char *code));
    virtual ~Command();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_COMMAND_H_
