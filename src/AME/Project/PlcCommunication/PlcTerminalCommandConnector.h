/*
 setVerboseTerminal * PlcTerminalCommandConnector.h
 *
 *  Created on: 10.06.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_PLCTERMINALCOMMANDCONNECTOR_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_PLCTERMINALCOMMANDCONNECTOR_H_

#include "../HelpStructures/List.h"
#include "ModemController.h"
#include "../HelpStructures/BitMask.h"
#include "IoCommandManager.h"
#include "../SystemBase/ProgramConfig.h"     // PLCF-233

namespace AME_SRC {

class PlcTerminalCommandConnector {
 public:
    PlcTerminalCommandConnector();
    virtual ~PlcTerminalCommandConnector();
    typedef struct {
        uint16_t val1;
        uint16_t val2;
    } containerFor2Values;

    static void addCmdTab2List();
    static char* sendMessageToNetworkMemberIndex(char *text);
    static char* pathDiscovery(char *text);
    static char* remotePathDiscovery(char *text);
    static char* getEAPAttribute(char *text);
    static char* getADPAttribute(char *text);
    static char* getMacAttribute(char *text);
    static char* setMacAttribute(char *text);
    static char* setADPAttribute(char *text);
    static char* setEAPAttribute(char *text);
    static char* setClientInfo(char *text);
    static char* getSystemVersionCmd(char *dummy);
    static char* callInitChannelWithMode(char *mode);
    static char* setVerboseTerminal(char *text);
    static char* join(char *text);
    static char* leave(char *text);
    static char* forcejoin(char *text);
    static char* forceNetworkMember(char *text);
    static char* searchNetwork(char *text);
    static char* setAliveNeighborGroupes(char *text);
    static char* startAliveCheck(char *text);
    static char* setAliveNeighborPeers(char *text);
    static char* lineEntryTestFkn(char *text);
    static char* remoteSearchNetwork(char *text);
    static char* triggerAutomaticSteps(char *text);
    static char* blockStoreTable(char *text);
    static char* getPLCAttributesStart(char *text);
    static char* setFCCoordCallOuts(char *text);
    static char* setFCCoordDelay(char *text);
    static char* setFCCoordScale(char *text);
    static char* testMalloc(char *text);
    static char* clearDeviceFC(char *text);
    static char* registerVirtualPeers(char *text);
    static char* pollPeer(char *text);
    static char* inputPeer(char *text);
    static char* outputPeer(char *text);
    static char* sendGlobalTimeout(char *text);
    static void doSendGlobalTimeout();
    static containerFor2Values* create2IntContainerFromPLCString(
            const char *str);
    static const char* skipPlcCmdHeader(const char *str);
    static char* terminalStartCmd(char *text);
    static char* terminalStopCmd(char *text);

 private:
    static char* sendIoCmd2Peer(uint16_t targetIP, const char cmd,
            IoCommandManager::ioSubCmds subCmd);
    static uint8_t RequestArgData[16];  // Container for setter function for example routing table

    static void getFirst2IntegerFromString(char *str,
            containerFor2Values *container);
    static void getRequestArgsFromString(char *str,
            ModemController::getRequestArgs *container);
    static char* getRSearchErrorMessage(uint16_t peerIP, uint8_t searchTime);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_PLCTERMINALCOMMANDCONNECTOR_H_
