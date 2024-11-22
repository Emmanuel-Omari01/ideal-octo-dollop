/*
 * pollingTrigger.cpp
 *
 *  Created on: 31.05.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "TriggerOperations.h"

namespace triggerOperations {

bool isNumberChangedOnceInNCalls(uint16_t number, uint16_t callMargin) {
    static uint16_t previousNumber = number;
    static uint16_t callCounter = callMargin;
    bool isNumberChanged = number != previousNumber;
    if (callCounter < callMargin) {
        callCounter++;
    } else {
        if (isNumberChanged) {
            callCounter = 0;
            return true;
        }
    }
    previousNumber = number;
    return false;
}

bool isNumberChangeForNCalls(const uint16_t number, uint16_t callMargin) {
    static uint16_t previousNumber = number;
    static uint16_t callCounter = callMargin;
    bool isNumberChanged = number != previousNumber;
    if (isNumberChanged) {
        callCounter = 0;
    } else {
        return ++callCounter == callMargin;
    }
    previousNumber = number;
    return false;
}

}  // namespace triggerOperations

