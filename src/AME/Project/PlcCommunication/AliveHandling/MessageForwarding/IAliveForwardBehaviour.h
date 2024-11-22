/*
 * IAliveForwardingBehaviour.h
 *
 *  Created on: Aug 4, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_IALIVEFORWARDBEHAVIOUR_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_IALIVEFORWARDBEHAVIOUR_H_

#include "AliveData.h"
#include "../../../SystemBase/TimeMeasurement.h"

namespace AME_SRC {

class IAliveForwardBehaviour {  // @suppress("Class has a virtual method and non-virtual destructor")
 public:
    virtual void forward(AliveData &incomingAlive) = 0;
    virtual void handleTransmissionFailure() = 0;
    virtual void start() = 0;

    virtual void setEnable(bool status)= 0;
    virtual bool getEnableStatus() = 0;

    static bool isMessageReceived() {
        return messageReceived;
    }

    void setMessageReceived(bool status) {
        this->messageReceived = status;
    }

    static bool messageReceived;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_IALIVEFORWARDBEHAVIOUR_H_
