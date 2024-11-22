//
// Created by plcdev on 25.06.24.
//

#include "SoftwareDeploymentFlow.h"
#include <stddef.h>

namespace AME_SRC {

SoftwareDeploymentFlow::deploySteps SoftwareDeploymentFlow::_deployStep = kIdle;
SoftwareDeploymentFlow::SoftwareDeploymentFlow(ITransmitter *transmitPtr,
        uint8_t *softawreCodePtr, DelayHandler delayHandler, uint8_t globComID,
        IEventStateProvider *stateProvider) :
        _transmitter(transmitPtr), _globComID(globComID), _delayHandler(
                delayHandler), softawreCodePtr_(softawreCodePtr), _isStartVerfied(
                false), _stateProvider(stateProvider) {
}

void SoftwareDeploymentFlow::proceed() {
    const uint8_t startMessage[6] = { ':', ':', 'U', ':', 'S', '\0' };
    char options[5] = { 0 };
    static uint32_t idxCounter = 0;
    switch (_deployStep) {
    case kStart:
        _transmitter->transmitt(startMessage, 6, _globComID, 0);
        _deployStep = kWaitUpdateProcessed;  // kPollStartAnswer;
        break;
    case kPollStartAnswer:

        if (_isStartVerfied) {
            _deployStep = kSendUpdate;
        }
        break;
    case kCopy:
        _updateMessage = UpdateMessage(UpdateType::fragment, idxCounter,
                idxCounter * 50, 50, softawreCodePtr_);
        // _updateMessage
        // _transmitter->transmitt(softawreCodePtr_, (uint8_t)50, _globComID, options);
        break;
    case kSendUpdate:
        if (idxCounter > 5) {
            _delayHandler.resetTimer();
            idxCounter = 0;
            _deployStep = kIdle;
        } else {
            idxCounter++;
            _delayHandler.resetTimer();
            _transmitter->transmitt(softawreCodePtr_ + idxCounter, (uint8_t) 50,
                    _globComID, options);
            _deployStep = kWaitUpdateProcessed;
        }
        break;
    case kWaitUpdateProcessed:
        if (_delayHandler.hasWaitNTimes(3)) {
            _delayHandler.resetTimer();
            _deployStep = kSendUpdate;
        }
        break;
    case kIdle:
        break;
    }
}

void SoftwareDeploymentFlow::start() {
    using states = IEventStateProvider::states;
    states systemState = _stateProvider->getState();
    bool isSystemStateValid = (systemState == states::kIdle);
    if (isSystemStateValid) {
        _deployStep = kStart;
    }
}

void SoftwareDeploymentFlow::stop() {
    _deployStep = kIdle;
}

void SoftwareDeploymentFlow::setFlowState(enum deploySteps step) {
    _deployStep = step;
}

void SoftwareDeploymentFlow::handleUpdateMessage(UpdateMessage uMessage) {
    const uint8_t startAnswer[6] = { ':', ':', 'u', ':', 's', '\0' };
    switch (uMessage.getType()) {
    case UpdateType::start:
        _transmitter->transmitt(startAnswer, 6, _globComID, 0);
        break;
    case UpdateType::startReply:
        uMessage.getId();
        break;
    }
}

}  // namespace AME_SRC
