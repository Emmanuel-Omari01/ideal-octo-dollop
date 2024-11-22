/*
 * MainLoop.h
 *
 *  Created on: 06.01.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_MAINLOOP_H_
#define SRC_AME_PROJECT_SYSTEMBASE_MAINLOOP_H_

#include <stdint.h>

namespace AME_SRC {

class MainLoop {
 private:
    static uint32_t microSecCounterLoop;
    static uint16_t microSecCounter100;
    static uint16_t milliSecCounter;
    static uint16_t milliSecCounter5;
    static uint16_t milliSecCounter50;
    static uint8_t secCounter;
    static bool hasStarted;
    void initOSTimer();
    bool counterIsMultipleOfTime(uint32_t counter, uint32_t time);
    void executePtrIfNotNull(void (*&executionPtr)());
    void (*execution100microPtr)();
    void (*execution1msPtr)();
    void (*execution5msPtr)();
    void (*execution50msPtr)();
    void (*execution1sPtr)();
    void (*execution60sPtr)();

 public:
    static uint16_t counterFlag;
    MainLoop();
    virtual ~MainLoop();
    static void start();
    static void stop();
    void executeUntilStop();
    static bool isStarted();

    static void incrementCounters();
    void setExecution50msPtr(void (*execution50msPtr)()) {
        this->execution50msPtr = execution50msPtr;
    }
    void setExecution1sPtr(void (*execution1sPtr)()) {
        this->execution1sPtr = execution1sPtr;
    }
    void setExecution1msPtr(void (*execution1msPtr)()) {
        this->execution1msPtr = execution1msPtr;
    }

    void setExecution5msPtr(void (*execution5msPtr)()) {
        this->execution5msPtr = execution5msPtr;
    }

    void setExecution60sPtr(void (*execution60sPtr)()) {
        this->execution60sPtr = execution60sPtr;
    }


    void setExecution1microPtr(void (*execution1microPtr)()) {
        this->execution100microPtr = execution1microPtr;
    }

    static uint32_t getLoopTime() {
        return microSecCounterLoop;
    }

    static void setLoopTime(uint32_t time) {
      microSecCounterLoop = time;
    }
};

}  // namespace AME_SRC

extern "C" void osTimerInterrupt();

#endif  // SRC_AME_PROJECT_SYSTEMBASE_MAINLOOP_H_
