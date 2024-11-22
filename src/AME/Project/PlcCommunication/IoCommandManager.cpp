/*
 * IoCommandManager.cpp
 *
 *  Created on: Feb 5, 2024
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "IoCommandManager.h"

#include <cstring>
#include <cstdlib>

#include "../SystemBase/ProgramConfig.h"
#include "PlcTerminalCommandConnector.h"

namespace AME_SRC {

IoCommandManager::ioSubCmds IoCommandManager::pollRequestState_ = verifyPoll;
uint8_t IoCommandManager::inDisplayIndex_ = 0xFF;
TermContextFilter *IoCommandManager::termFilterPtr_ = NULL;
ITimeOutWatcher *IoCommandManager::watcher_ = 0;
char IoCommandManager::paramString_[12] = "000:000:000";

#pragma GCC diagnostic ignored "-Wstack-usage="
IoCommandManager::IoCommandManager(ITimeOutWatcher &timeOutWatcher,
        SerialDriver *serialDriver, LEDSignalFlow &ledSignalFlowRef,
        IoPinGroup &ioInputs, IoPinGroup &ioOutputs) :
        ledSignalFlowRef_ { ledSignalFlowRef }, ioInputs_ { ioInputs }, ioOutputs_ {
                ioOutputs }, isPolling_ { false } {
    watcher_ = &timeOutWatcher;
    termFilterPtr_ = new TermContextFilter(serialDriver);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    ITimeOutWatcher::timeOutInfo *toi;
    toi = ITimeOutWatcher::createOneCallTimeOut(
        ProgramConfig::cmdRetransmittTOutID, sendInBuffToTargetIP, informUserAboutCmdCallout);
    watcher_->registerTimeout(*toi);
    watcher_->stopAndReset(ProgramConfig::cmdRetransmittTOutID);
}

IoCommandManager::~IoCommandManager() = default;

void IoCommandManager::sendIoCmdRequest(uint16_t ip, const char cmd, enum ioSubCmds subCmd) {
    const char *messageCmdIdentifier = 0;
    switch (cmd) {
        case ModemInterpreter::PollRequest:
            messageCmdIdentifier = "Poll";
            break;
        case ModemInterpreter::InputRequest:
            messageCmdIdentifier = "Input";
            break;
        case ModemInterpreter::OutputRequest:
            messageCmdIdentifier = "Output";
            break;
        case ModemInterpreter::SwitchMatrixRequest:
            messageCmdIdentifier = "Switch";
            break;
        default:
            termFilterPtr_->println(TX::IoCmdWrongInput);
            return;
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::IOCmdRequestWasSendToIP), messageCmdIdentifier, ip);
    termFilterPtr_->println(Global2::OutBuff);
    sendSupervisedIoCmd(cmd, subCmd, ip);
}

void IoCommandManager::answerPollRequest(const char *subCmd) {
    switch (subCmd[0]) {
        case startPoll:
            isPolling_ = true;
            break;
        case stopPoll:
            isPolling_ = false;
            break;
        case verifyPoll:
            break;
    }
    reactOnPollRequest();
}


void IoCommandManager::reactOnPollRequest() {
    createPollReplyBody();
    frameAndTransmitWithCmd(ModemInterpreter::pollReply);
    informUserAboutPollStatus();
}

void IoCommandManager::createPollReplyBody() {
    Global2::InBuff[0] = 0;
    snprintf(Global2::InBuff, Global2::inBuffMaxLength, "%d", isPolling_);
}


void IoCommandManager::informUserAboutPollStatus() {
    LEDSignalFlow::eLEDMode pollLEDStatus =
            isPolling_ ? LEDSignalFlow::poll : LEDSignalFlow::connected;
    ledSignalFlowRef_.setLedMode(pollLEDStatus);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::IoCmdPollState),
            (isPolling_ ? TX::getText(TX::StateActive) : TX::getText(TX::StateStopped)));
    termFilterPtr_->println(Global2::OutBuff);
}

void IoCommandManager::answerPinStatusWithReplyType(char replyType) {
    IoPinGroup *groupPtr = 0;
    if (replyType == ModemInterpreter::InputReply) {
        groupPtr = &ioInputs_;
        termFilterPtr_->print("Input:", TermContextFilter::filterPLCText);
    } else if (replyType == ModemInterpreter::outputReply) {
        groupPtr = &ioOutputs_;
        termFilterPtr_->print("Output:", TermContextFilter::filterPLCText);
    } else {
        termFilterPtr_->println(TX::getText(TX::IOCmdWrongPinStateReplyType),
                TermContextFilter::filterPLCText);
        return;
    }
    if (groupPtr) {
        createReplyBodyForPinGroup(*groupPtr);
    } else {
        Global2::InBuff[0] = 0;
    }
    frameAndTransmitWithCmd(replyType);
    termFilterPtr_->println(TX::IOCmdRequestReplied,
            TermContextFilter::filterPLCText);
}


void IoCommandManager::reactOnOutputReqeuest(char *subCmd) {
    PlcTerminalCommandConnector::containerFor2Values *outPutParamPtr =
            PlcTerminalCommandConnector::create2IntContainerFromPLCString(subCmd);
    uint16_t outPutIndex = outPutParamPtr->val1;
    uint16_t outPutLevel = outPutParamPtr->val2;
    if ((outPutIndex == fillerParam) || (outPutLevel == fillerParam)) {
        return;
    } else if (outPutIndex > ioOutputs_.getPinAmount()) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::IOCmdOutputIndexWrong), outPutIndex);
        termFilterPtr_->println(Global2::OutBuff);
        return;
    }

    ioOutputs_[outPutIndex]->setLevel((PortManager::pinLevel) outPutLevel);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::IoCmdOutputSetTo),
            outPutIndex, (outPutLevel ? TX::getText(TX::txtHigh) : TX::getText(TX::txtLow)));
    termFilterPtr_->println(Global2::OutBuff);
    delete outPutParamPtr;
}

void IoCommandManager::createReplyBodyForPinGroup(IoPinGroup &pinGroup) {
    Global2::InBuff[0] = 0;
    for (int pinIndex = 0; pinIndex < pinGroup.getPinAmount(); ++pinIndex) {
        appendPinGroupIndexLevelToInBuff(pinGroup, (uint8_t) pinIndex);
    }
}

void IoCommandManager::sendSupervisedIoCmd(char cmdLetter, char subCmdLetter, uint16_t targetIP) {
    snprintf(Global2::InBuff, Global2::inBuffMaxLength, "%2s%c%03d:%c:%s", ModemInterpreter::getCommandPrefix(),
            cmdLetter, targetIP, subCmdLetter, paramString_);

    // ITimeOutWatcher::timeOutInfo *pToi = watcher_->findTimeOutWithID(ProgramConfig::cmdRetransmittTOutID);
    watcher_->start(ProgramConfig::cmdRetransmittTOutID);
    termFilterPtr_->println("|---|");
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t*>(Global2::InBuff), (uint8_t) strlen(Global2::InBuff),
            IRoutingManager::BCGlobalIP, true, 0);
}



void IoCommandManager::informUserAboutCmdCallout() {
    termFilterPtr_->println("|");
    termFilterPtr_->println(TX::IoCmdRequestNotReplied);
}


void IoCommandManager::sendInBuffToTargetIP() {
    ITimeOutWatcher::timeOutInfo *retransmitTOut = watcher_->findTimeOutWithID(ProgramConfig::cmdRetransmittTOutID);
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t*>(Global2::InBuff), (uint8_t) strlen(Global2::InBuff),
            IRoutingManager::BCGlobalIP, true, 0);
    bool isFirstTime = (retransmitTOut->timeOutCounter == (retransmitTOut->timeOutAmount - 1));
    if (isFirstTime) {
        termFilterPtr_->print("|#");
    } else {
        termFilterPtr_->putchar('#');
    }
}


void IoCommandManager::frameAndTransmitWithCmd(char cmdLetter) {
    const uint8_t coordinatorIP = 0;
    snprintf(paramString_, sizeof(paramString_), "%s%c%03d:%s", ModemInterpreter::commandPrefix,
            cmdLetter, coordinatorIP, Global2::InBuff);
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t*>(paramString_), (uint8_t) strlen(paramString_),
            IRoutingManager::BCGlobalIP, true, 0);
}


void IoCommandManager::appendPinGroupIndexLevelToInBuff(IoPinGroup &pinGroup, uint8_t index) {
    char tmp[4];
    PortManager::pinLevel pinLevel = pinGroup[index]->getLevel();
    snprintf(tmp, sizeof(tmp), "%u:", pinLevel);
    strncat(Global2::InBuff, tmp, sizeof(Global2::InBuff) - strlen(Global2::InBuff) - 1);
}


char* IoCommandManager::transformInOrOutReply(char *reply, bool isInput) {
    enum { DisplayAll = 0XFF };
    const char *replyDisplayFormat;
    if (isInput) {
        replyDisplayFormat = TX::getText(TX::IoCmdInputState);
    } else {
        replyDisplayFormat = TX::getText(TX::IoCmdOutputState);
    }

    watcher_->stopAndReset(ProgramConfig::cmdRetransmittTOutID);
    uint8_t index = 1;
    reply += CMDReplyOffset_;
    char *savePtr;
    char *token = strtok_r(reply, ":", &savePtr);
    Global2::OutBuff[0] = 0;
    do {
        if (inDisplayIndex_ != DisplayAll) {
            if (index == inDisplayIndex_) {
                appendInputDisplayTextToOutBuf(index, token, replyDisplayFormat);
                break;
            }
        } else {
            appendInputDisplayTextToOutBuf(index, token, replyDisplayFormat);
        }
        index++;
        token = strtok_r(NULL, ":", &savePtr);
    } while (token != NULL);
    termFilterPtr_->println("|");
    return Global2::OutBuff;
}

void IoCommandManager::appendInputDisplayTextToOutBuf(uint8_t index,
        char *inputToken, const char *replyDisplayFormat) {
    char outPutContainer[16];
    uint8_t inputPrefixId = (index > 2 ? 2 : 1);
    uint8_t inputSuffixId = ((index % 2) == 0 ? 2 : 1);
    snprintf(outPutContainer, sizeof(outPutContainer), replyDisplayFormat, inputPrefixId, inputSuffixId,
            atoi(inputToken));
    strncat(Global2::OutBuff, outPutContainer, Global2::inBuffMaxLength);
}


char* IoCommandManager::transformPollReply(char *reply) {
    ITimeOutWatcher::timeOutInfo *reTransmitTout = watcher_->findTimeOutWithID(
        ProgramConfig::cmdRetransmittTOutID);

    if (reTransmitTout) {
        uint8_t timeoutDelta = uint8_t(
                reTransmitTout->timeOutAmount - reTransmitTout->timeOutCounter);
        char loadBarEnding[5];
        memset(loadBarEnding, '-', timeoutDelta);
        loadBarEnding[timeoutDelta] = '|';
        termFilterPtr_->print(loadBarEnding);
        watcher_->stopAndReset(ProgramConfig::cmdRetransmittTOutID);
        reply += CMDReplyOffset_;
        bool isPolling = atoi(reply);
        uint16_t lastSrcIP = ModemInterpreter::getLastDataIndicationSrcIp();
        const char *pollStatusMessage =
                isPolling ? TX::getText(TX::txtExecuted) : TX::getText(TX::txtNotExecuted);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::IoCmdExecutePoll),
                pollStatusMessage, lastSrcIP);
        return Global2::OutBuff;
    } else {
        return const_cast<char *>(TX::getText(TX::IoCmdReplyWithNoRequest));
    }
}

char* IoCommandManager::transformSwitchReply(__attribute__((unused))char *reply) {
    ITimeOutWatcher::timeOutInfo *reTransmitTout = watcher_->findTimeOutWithID(ProgramConfig::cmdRetransmittTOutID);
    if (reTransmitTout) {
        watcher_->stopAndReset(ProgramConfig::cmdRetransmittTOutID);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::IoCmdSwitchACK));
        return Global2::OutBuff;
    } else {
        return const_cast<char *>(TX::getText(TX::IoCmdSwitchReplyNoRequest));
    }
}

char* IoCommandManager::transformSwitchRequest(char *request) {
    enum { switchMatrixCmdOffset = 19, subCMDOffset = 2 };
    request += (CMDReplyOffset_ + subCMDOffset);
    int inputNumb = atoi(request);
    bool isLow = inputNumb > 23;
    inputNumb -= switchMatrixCmdOffset;
    uint16_t targetIP = ModemInterpreter::getLastDataIndicationSrcIp();
    snprintf(Global2::InBuff, Global2::inBuffMaxLength, "%2s%c%03d:%c:%s",
            ModemInterpreter::getCommandPrefix(),
            ModemInterpreter::switchMatrixReply, targetIP, noCMD, paramString_);
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t*>(Global2::InBuff), (uint8_t) strlen(Global2::InBuff),
            IRoutingManager::BCGlobalIP, true, 0);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::IoCmdInputReportMsg), targetIP,
            isLow ? inputNumb - 4 : inputNumb, isLow ? TX::getText(TX::txtLow) : TX::getText(TX::txtHigh));
    return Global2::OutBuff;
}

}  // namespace AME_SRC
