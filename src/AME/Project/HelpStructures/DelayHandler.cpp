/*
 * DelayHandler.cpp
 *
 *  Created on: 06.09.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "DelayHandler.h"

namespace AME_SRC {

int DelayHandler::waitCount = 0;

DelayHandler::DelayHandler() = default;

DelayHandler::~DelayHandler() = default;

bool DelayHandler::hasWaitNTimes(uint32_t N) {
    if (waitCount++ > N) {
        waitCount = 0;
        return true;
    } else {
        return false;
    }
}

void DelayHandler::resetTimer() {
    waitCount = 0;
}

}  // namespace AME_SRC
