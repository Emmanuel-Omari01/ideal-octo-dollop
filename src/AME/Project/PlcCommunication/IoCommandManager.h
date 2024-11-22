/*
 * IoCommandManager.h
 *
 *  Created on: Feb 5, 2024
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_IOCOMMANDMANAGER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_IOCOMMANDMANAGER_H_

#include <cstdint>
#include <cstdio>

#include "../CompositeComponents/FoundationModule/ITimeOutWatcher.h"
#include "../Terminal/SerialDrivers/SerialDriver.h"
#include "../SignalProcessing/Aktorik/LED/LEDSignalFlow.h"
#include "../SignalProcessing/IoPinHandling/IoPin.h"
#include "../SignalProcessing/IoPinHandling/IoPinGroup.h"
#include "../Terminal/TermContextFilter.h"

namespace AME_SRC {
class IoCommandManager {
 public:
    enum ioSubCmds {
        startPoll = 'S',
        stopPoll = 's',
        verifyPoll = 'V',
        getOutput = 'G',
        setOutput = 'S',
        noCMD = 'N'
    };
    enum {
        fillerParam = 0xFF
    };
    IoCommandManager(ITimeOutWatcher &timeOutWatcher, SerialDriver *serialDriver, LEDSignalFlow &ledSignalFlowRef,
            IoPinGroup &ioInputs, IoPinGroup &ioOutputs);
    virtual ~IoCommandManager();

    static void sendIoCmdRequest(uint16_t ip, const char cmd, enum ioSubCmds subCmd);

    void answerPollRequest(const char* subCmd);
    void answerPinStatusWithReplyType(char replyType);
    void reactOnOutputReqeuest(char* subCmd);

    char* transformInOrOutReply(char* reply, bool isInput);
        void appendInputDisplayTextToOutBuf(uint8_t index, char* inputToken, const char* replyDisplayFormat);

    char* transformPollReply(char* reply);

    char* transformSwitchReply(char* reply);
    char* transformSwitchRequest(char* request);

    static void set3ParamForIoCmd(uint8_t param1, uint8_t param2, uint8_t param3) {
        snprintf(paramString_, sizeof(paramString_), "%03u:%03u:%03u", param1, param2, param3);
    }

    static void sendSupervisedIoCmd(char cmdLetter, char subCmdLetter, uint16_t targetIP);

    static void setInputDisplayIndex(uint8_t inDisplayIndex) {
        inDisplayIndex_ = inDisplayIndex;
    }

    static enum ioSubCmds getPollRequestState() {
        return pollRequestState_;
    }

    static void setPollRequestState(
            IoCommandManager::ioSubCmds pollRequestState) {
        pollRequestState_ = pollRequestState;
    }

 private:
    static char paramString_[12];
    static ITimeOutWatcher *watcher_;
    static TermContextFilter *termFilterPtr_;
    LEDSignalFlow &ledSignalFlowRef_;
    IoPinGroup &ioInputs_;
    IoPinGroup &ioOutputs_;
    const uint8_t CMDReplyOffset_ = 5;
    bool isPolling_;

    static enum ioSubCmds pollRequestState_;

    static uint8_t inDisplayIndex_;

    void createPollReplyBody();

    void createReplyBodyForPinGroup(IoPinGroup &pinGroup);

    void appendPinGroupIndexLevelToInBuff(IoPinGroup& pinGroup, uint8_t index);
    void frameAndTransmitWithCmd(char cmdLetter);

    static void sendInBuffToTargetIP();
    static void informUserAboutCmdCallout();
    void reactOnPollRequest();
    void informUserAboutPollStatus();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_IOCOMMANDMANAGER_H_
