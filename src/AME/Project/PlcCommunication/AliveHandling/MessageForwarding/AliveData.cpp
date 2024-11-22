/*
 * AliveData.cpp
 *
 *  Created on: Aug 4, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "AliveData.h"

namespace AME_SRC {

const  char AliveData::alivePrefix[4] = "::A";

AliveData::AliveData(const enum Direction inout, uint16_t inoutGroup, uint8_t memberID) :
        status_ { Alive_Unknown } {
    if (inout == incoming) {
        incomingGroup_ = inoutGroup;
        outgoingGroup_ = 0;
    } else {
        outgoingGroup_ = inoutGroup;
        incomingGroup_ = 0;
    }
    data_.prefix = alivePrefix;
    data_.dataField.all = 0;
    data_.memberID = memberID;
}

void AliveData::incrementPassedNodes() {
    data_.dataField.bit.passedNodes++;
}


AliveData::~AliveData() = default;

}  // namespace AME_SRC
