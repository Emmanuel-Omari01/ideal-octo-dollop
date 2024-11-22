/*
 * CompareTimer1.h
 *
 *  Created on: 09.11.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPARETIMER1_H_
#define SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPARETIMER1_H_

#include "CompareMatchTimeDriver.h"

namespace AME_SRC {

class CompareTimer1: public CompareMatchTimeDriver {
 private:
    void create();
 public:
    void start();
    void stop();
    explicit CompareTimer1(uint16_t delay);
    CompareTimer1(uint16_t MikroDelay, void (*funcName)());
    virtual void setMatchInterruptFuncPtr(void (*funcName)());
    virtual ~CompareTimer1();
    static void (*matchInterruptPtrChannel1)();
};

}  // namespace AME_SRC

extern "C" void timerInterruptChannel1();

#endif  // SRC_AME_PROJECT_SYSTEMBASE_DRIVER_COMPARETIMER1_H_
