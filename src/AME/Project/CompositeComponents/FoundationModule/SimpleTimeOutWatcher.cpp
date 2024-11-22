/*
 * SimpleTimeOutWatcher.cpp
 *
 *  Created on: 02.06.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "SimpleTimeOutWatcher.h"

#include "../../SystemBase/Global.h"

namespace AME_SRC {

SimpleTimeOutWatcher::SimpleTimeOutWatcher() {
    for (int i=0; i < static_cast<int>(sizeof(timeOutContainers)/sizeof(timeOutInfo)); i++) {
        timeOutContainers[i].id = 0;
        timeOutContainers[i].started = false;
    }
}

SimpleTimeOutWatcher::~SimpleTimeOutWatcher() = default;

void SimpleTimeOutWatcher::start(uint8_t id) {
    timeOutInfo *toBeStartedPtr = findTimeOutWithID(id);
    if (toBeStartedPtr) {
        toBeStartedPtr->started = true;
    }
}

void SimpleTimeOutWatcher::stopAndReset(uint8_t id) {
    timeOutInfo *toBeStopedPtr = findTimeOutWithID(id);
    if (toBeStopedPtr) {
        toBeStopedPtr->started = false;
        toBeStopedPtr->timeCounter = toBeStopedPtr->delay;
        toBeStopedPtr->timeOutCounter = toBeStopedPtr->timeOutAmount;
        toBeStopedPtr->callOutCounter = toBeStopedPtr->callOutAmount;
    }
}

char* SimpleTimeOutWatcher::getStatus() {
    return nullptr;
}

void SimpleTimeOutWatcher::update() {
    for (int i=0; i < static_cast<int>(sizeof(timeOutContainers)/sizeof(timeOutInfo)); i++) {
        if (timeOutContainers[i].id != 0 && timeOutContainers[i].started) {
            updateTimeOutInfo(timeOutContainers[i]);
        }
    }
}

void SimpleTimeOutWatcher::updateTimeOutInfo(timeOutInfo &info) {
    if (info.timeCounter-- < 1) {
        info.timeCounter = info.delay;
        if (info.timeOutCounter-- < 1) {
            if (info.callOutCounter-- < 1) {
                info.started = false;
                info.timeCounter = info.delay;
                info.timeOutCounter = info.timeOutAmount;
                info.callOutCounter = info.callOutAmount;  // info.callOutAmount; // 01.12.2023
            } else {
                info.timeOutCounter = info.timeOutAmount;
            }
            if (info.callOutFunc) {
                info.callOutFunc();
            }
        } else {
            if (info.timeOutFunc) {
                info.timeOutFunc();
            }
        }
    }
}

void SimpleTimeOutWatcher::registerTimeout(timeOutInfo &info) {
    setTimeOutInfoAtID(info.id, info.timeOutCounter, info.callOutCounter,
            info.timeOutAmount, info.callOutAmount, info.delay,
            info.timeOutFunc, info.callOutFunc, info.started, 0);
}

void SimpleTimeOutWatcher::clearTimeout(uint8_t index) {
    setTimeOutInfoAtID(0, 0, 0, 0, 0, 0, nullptr, nullptr, false, index);
}

bool SimpleTimeOutWatcher::editTimeoutdelayForID(uint8_t searchedID,
                                                 uint8_t newDelay) {
    timeOutInfo *timeOutInfoPtr = findTimeOutWithID(searchedID);
    if (timeOutInfoPtr) {
        timeOutInfoPtr->delay = newDelay;
        timeOutInfoPtr->timeCounter = timeOutInfoPtr->delay;
        return true;
    }
    return false;
}

void SimpleTimeOutWatcher::resetDelayForIndex(uint8_t index) {
    timeOutInfo *infoPtr = findTimeOutWithID(index);
    if (infoPtr) {
        infoPtr->timeCounter = infoPtr->delay;
        infoPtr->timeOutCounter = infoPtr->timeOutAmount;
    }
}

ITimeOutWatcher::timeOutInfo* SimpleTimeOutWatcher::findTimeOutWithID(uint8_t searchedID) {
    for (int i=0; i < static_cast<int>(sizeof(timeOutContainers)/sizeof(timeOutInfo)); i++) {
        if (timeOutContainers[i].id == searchedID) {
            return &timeOutContainers[i];
        }
    }
    return nullptr;
}

void SimpleTimeOutWatcher::setTimeOutInfoAtID(uint8_t id, int8_t timeOuts,
                                              int8_t callOuts, int8_t timeOutAmount, int8_t callOutAmount,
                                              uint16_t delay, void (*timeOutFunc)(), void (*callOutFunc)(),
                                              bool started, uint8_t searchedID) {
    timeOutInfo *infoPtr = findTimeOutWithID(searchedID);
    if (infoPtr) {
        infoPtr->callOutFunc = callOutFunc;
        infoPtr->callOutCounter = callOuts;
        infoPtr->callOutAmount = callOutAmount;
        infoPtr->id = id;
        infoPtr->started = started;
        infoPtr->timeOutFunc = timeOutFunc;
        infoPtr->timeOutAmount = timeOutAmount;
        infoPtr->timeOutCounter = timeOuts;
        infoPtr->delay = delay;
        infoPtr->timeCounter = delay;
    }
}

}  // namespace AME_SRC
