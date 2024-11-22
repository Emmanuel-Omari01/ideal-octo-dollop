/*
 * triggerOperationsTrigger.h
 *
 *  Created on: 31.05.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_TRIGGEROPERATIONS_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_TRIGGEROPERATIONS_H_

#include <cstdint>

namespace triggerOperations {

bool isNumberChangedOnceInNCalls(uint16_t number, uint16_t callMargin);
bool isNumberChangeForNCalls(const uint16_t number, uint16_t callMargin);

}  // namespace triggerOperations

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_TRIGGEROPERATIONS_H_
