/* Copyright (C) 2024 Tobias Hirsch - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the XYZ license.
 */

#ifndef SRC_AME_PROJECT_UPDATEMODULE_SOFTWAREDEPLOYMENTFLOW_H_
#define SRC_AME_PROJECT_UPDATEMODULE_SOFTWAREDEPLOYMENTFLOW_H_

#include "ITransmitter.h"
#include "IUpdateTranslator.h"
#include "UpdateMessage.h"
#include "../HelpStructures/DelayHandler.h"
#include "UpdateDependencies.h"

namespace AME_SRC {

class SoftwareDeploymentFlow {
 public:
    explicit SoftwareDeploymentFlow(ITransmitter *transmitPtr,
            uint8_t *softawreCodePtr, DelayHandler delayHandler,
            uint8_t globComID, IEventStateProvider *stateProvider);
    enum deploySteps {
        kIdle,
        kStart,
        kPollStartAnswer,
        kCopy,
        kSendUpdate,
        kWaitUpdateProcessed,
        kSendCheck,
        kPollCheckAnswer,
        kProcessFragment,
    };
    void proceed();
    void start();
    void stop();
    void setFlowState(enum deploySteps step);
    void handleUpdateMessage(UpdateMessage uMessage);

    enum deploySteps getDeployStep() const {
        return _deployStep;
    }

 private:
    uint16_t _globComID;
    static enum deploySteps _deployStep;
    ITransmitter *_transmitter;
    bool _isStartVerfied;
    const uint8_t *softawreCodePtr_;
    DelayHandler _delayHandler;
    UpdateMessage _updateMessage;
    IEventStateProvider* _stateProvider;
};
}  // namespace AME_SRC
#endif  // SRC_AME_PROJECT_UPDATEMODULE_SOFTWAREDEPLOYMENTFLOW_H_
