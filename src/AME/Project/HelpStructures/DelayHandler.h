/*
 * DelayHandler.h
 *
 *  Created on: 06.09.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_HELPSTRUCTURES_DELAYHANDLER_H_
#define SRC_AME_PROJECT_HELPSTRUCTURES_DELAYHANDLER_H_

#include <cstdint>

namespace AME_SRC {

class DelayHandler {
 public:
    DelayHandler();
    virtual ~DelayHandler();
    static bool hasWaitNTimes(uint32_t N);
    static void resetTimer();
    static int waitCount;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_HELPSTRUCTURES_DELAYHANDLER_H_
