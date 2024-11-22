/*
 * Switch.h
 *
 *  Created on: 07.12.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_SWITCH_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_SWITCH_H_

#include "IoPin.h"

namespace AME_SRC {

class Switch {
 private:
    const IoPin *triggerPinPtr;

 public:
    explicit Switch(const IoPin *pinPtr);
    virtual ~Switch();
    bool isLevelChanged();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_SWITCH_H_
