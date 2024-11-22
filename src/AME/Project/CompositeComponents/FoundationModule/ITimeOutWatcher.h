/*
 * ITimeOutWatcher.h
 *
 *  Created on: 02.06.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_ITIMEOUTWATCHER_H_
#define SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_ITIMEOUTWATCHER_H_

#include <cstdint>

namespace AME_SRC {

class ITimeOutWatcher {  // @suppress("Class has a virtual method and non-virtual destructor")
 public:
    typedef struct {
        bool started;
        uint8_t id;
        int8_t timeOutCounter;
        int8_t timeOutAmount;
        int8_t callOutCounter;
        int8_t callOutAmount;
        uint16_t timeCounter;
        uint16_t delay;
        void (*timeOutFunc)();
        void (*callOutFunc)();
    } timeOutInfo;

    static timeOutInfo* createDefaultTimeOut(uint8_t id,
            void (*timeOutFunc)(), void (*callOutFunc)()) {
        auto *infoPtr = new timeOutInfo();
        infoPtr->id = id;
        infoPtr->timeOutCounter = 3;
        infoPtr->timeOutAmount = 3;
        infoPtr->callOutCounter = 3;
        infoPtr->callOutAmount = 3;
        infoPtr->timeCounter = 120;
        infoPtr->delay = 120;
        infoPtr->callOutFunc = callOutFunc;
        infoPtr->timeOutFunc = timeOutFunc;
        infoPtr->started = true;
        return infoPtr;
    }

    static timeOutInfo* createOneCallTimeOut(uint8_t id,
            void (*timeOutFunc)(), void (*callOutFunc)()) {
        auto *infoPtr = new timeOutInfo();
        infoPtr->id = id;
        infoPtr->timeOutCounter = 3;
        infoPtr->timeOutAmount = 3;
        infoPtr->callOutCounter = 0;
        infoPtr->callOutAmount = 0;
        infoPtr->timeCounter = 120;
        infoPtr->delay = 120;
        infoPtr->callOutFunc = callOutFunc;
        infoPtr->timeOutFunc = timeOutFunc;
        infoPtr->started = true;
        return infoPtr;
    }

    // 2nd variation - without new statement
    static void createDefaultTimeOut(timeOutInfo *infoPtr, uint8_t id,
            void (*timeOutFunc)(), void (*callOutFunc)()) {
        infoPtr->id = id;
        infoPtr->timeOutCounter = 3;
        infoPtr->timeOutAmount = 3;
        infoPtr->callOutCounter = 3;
        infoPtr->callOutAmount = 3;
        infoPtr->timeCounter = 120;
        infoPtr->delay = 120;
        infoPtr->callOutFunc = callOutFunc;
        infoPtr->timeOutFunc = timeOutFunc;
        infoPtr->started = true;
    }

    static timeOutInfo* createTripleTimeOut(uint8_t id,
            void (*timeOutFunc)(), void (*callOutFunc)()) {
        auto *infoPtr = new timeOutInfo();
        infoPtr = createTripleTimeOut(infoPtr, id, timeOutFunc, callOutFunc);
        return infoPtr;
    }

    static timeOutInfo* createTripleTimeOut(timeOutInfo *infoPtr, uint8_t id,
            void (*timeOutFunc)(), void (*callOutFunc)()) {
        infoPtr->id = id;
        infoPtr->timeOutCounter = 30;   // before: 3
        infoPtr->timeOutAmount = 30;
        infoPtr->callOutCounter = 3;
        infoPtr->callOutAmount = 3;
        infoPtr->timeCounter = 10;
        infoPtr->delay = 10;
        infoPtr->callOutFunc = callOutFunc;
        infoPtr->timeOutFunc = timeOutFunc;
        infoPtr->started = false;
        return infoPtr;
    }

    virtual void start(uint8_t id) = 0;
    virtual void stopAndReset(uint8_t id) = 0;
    virtual char* getStatus() = 0;
    virtual void update() = 0;
    virtual void registerTimeout(timeOutInfo &info) = 0;
    virtual void clearTimeout(uint8_t index) = 0;
    virtual bool editTimeoutdelayForID(uint8_t searchedID, uint8_t newDelay) = 0;
    virtual void resetDelayForIndex(uint8_t index) = 0;
    virtual ITimeOutWatcher::timeOutInfo* findTimeOutWithID(uint8_t searchedID) = 0;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_ITIMEOUTWATCHER_H_
