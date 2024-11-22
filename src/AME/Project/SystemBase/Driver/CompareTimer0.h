/*
 * CompareTimer0.h
 *
 *  Created on: 02.06.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPARETIMER0_H_
#define SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPARETIMER0_H_

#include "CompareMatchTimeDriver.h"

namespace AME_SRC {

class CompareTimer0: public CompareMatchTimeDriver {
 private:
    void create();
 public:
    void start();
    void stop();
    explicit CompareTimer0(uint16_t delay);
    CompareTimer0(uint16_t MicroDelay, void (*funcName)());
    virtual ~CompareTimer0();
    static void (*matchInterruptPtrChannel0)();
    void setMatchInterruptFuncPtr(void (*funcName)());
};

extern "C" void timerInterruptChannel0();

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPARETIMER0_H_
