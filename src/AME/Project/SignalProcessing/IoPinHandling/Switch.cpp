/*
 * Switch.cpp
 *
 *  Created on: 07.12.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "Switch.h"

#include <cstdint>

namespace AME_SRC {

Switch::Switch(const IoPin *pinPtr) : triggerPinPtr(pinPtr) {
}

Switch::~Switch() = default;

bool Switch::isLevelChanged() {
    enum prellSteps {
        levelChange, wait, levelProof
    };
    static enum prellSteps prellStep = levelChange;
    static PortManager::pinLevel formerLevel = triggerPinPtr->getLevel();
    static uint8_t timeCount = 0;
    static uint8_t prellTime = 10;
    bool levelHasChanged;
    bool timeExpired;

    levelHasChanged = (formerLevel != triggerPinPtr->getLevel());
    switch (prellStep) {
    case levelChange:
        if (levelHasChanged) {
            prellStep = wait;
        }
        break;
    case wait:
        timeExpired = (timeCount++ > prellTime);
        if (timeExpired) {
            timeCount = 0;
            prellStep = levelProof;
        }
        break;
    case levelProof:
        prellStep = levelChange;
        if (!levelHasChanged) {
            return true;
        }
        break;
    }
    formerLevel = triggerPinPtr->getLevel();
    return false;
}

}  // namespace AME_SRC
