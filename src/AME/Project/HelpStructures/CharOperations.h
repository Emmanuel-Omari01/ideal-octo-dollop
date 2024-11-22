/*
 * charOperations.h
 *
 *  Created on: 09.07.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_CHAROPERATIONS_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_CHAROPERATIONS_H_

namespace charOperations {

unsigned char commandArgumentToNumber(char* command);
const char *skip_token(const char *code, char token);

}  // namespace charOperations

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_CHAROPERATIONS_H_
