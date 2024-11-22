/*
 // * ModemInterpreter.cpp
 *
 *  Created on: 25.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
*/

#include "ModemInterpreter.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <new>

#include "../../../Renesas/NotGenerated/crc32.h"
#include "G3PlcSystem/PlcCommand.h"
#include "PlcTerminalCommandConnector.h"
#include "../SystemBase/MainLoop.h"
#include "../SystemBase/MicrocontrollerConfig.h"
#include "G3PlcSystem/RequestFactory/RequestFactory.h"
#include "../HelpStructures/DelayHandler.h"
#include "../SystemLogic/StorageAdministrator.h"
#include "../HelpStructures/CharOperations.h"
#include  "AliveHandling/NeighbourAssignment/AssignmentFlow.h"
#include "../StorageManagement/SimpleFlashEditor.h"
#include "PlcCommandTypeSpecification.h"

namespace AME_SRC {
const char ModemInterpreter::commandPrefix[] = { "::" };
// using namespace PLC_CommandTypeSpecification;

bool ModemInterpreter::isNetworkSearchTriggerd = false;
uint8_t ModemInterpreter::modulID = 0;

uint32_t ModemInterpreter::roundTripTime = 0;
uint32_t ModemInterpreter::frameCounter = 0;
uint16_t ModemInterpreter::dataStatusCode = 0;
bool ModemInterpreter::dataStatusSet = false;
uint16_t ModemInterpreter::lastSwitchMessagePanIP = 0;
uint16_t ModemInterpreter::lastDataIndicationSrcIP_ = 0xFF;
uint16_t ModemInterpreter::deviceTableCurrentValue = 0;
uint16_t ModemInterpreter::deviceTableSearchValue = 0;
uint16_t ModemInterpreter::aliveReEntryPair[2] = { 0, 0 };
bool ModemInterpreter::isRouteFound = true;
bool ModemInterpreter::connected = false;
ModemInterpreter::sForceJoinState ModemInterpreter::ForceJoinState =
        ModemInterpreter::FJinit;
bool ModemInterpreter::softwareNotInStorage = false;
bool ModemInterpreter::macFrameReadCompleteStatus = true;
bool ModemInterpreter::peerRoutingProcess = false;
IDiscoveryManager *ModemInterpreter::discMangerPtr = NULL;

uint8_t ModemInterpreter::responsArray[ModemController::networkMemberMax + 1] =
        { 0 };
ModemInterpreter::routingState ModemInterpreter::lastRoutingState = routeIdle;

NetworkMember *ModemInterpreter::pingTargetPtr = 0;
ModemController *ModemInterpreter::modemControll = 0;
enum ModemInterpreter::InterpretState ModemInterpreter::interpretState = idle;
uint8_t ModemInterpreter::nextInterpretState = idle;
ModemTransmitter *ModemInterpreter::transmitter = 0;
ModemBootLoader *ModemInterpreter::modemBootloader = 0;
TermContextFilter *ModemInterpreter::termFilterPtr_ = NULL;

// List of members.
// Structure:
//        Peer: networkMemberList[0] - Route to coordinator
// coordinator: networkMemberList[1..<networkMemberMax>] - available peers

/*** DS-
 static const uint8_t peerMac[8][8]=
 {{ 0, 0, 0, 0, 0, 0, 0, 0},
 {0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x01},
 {0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x02},
 {0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x03},
 {0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x04},
 {0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x05},
 {0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x06},
 {0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x07}};
 ***/

#pragma GCC diagnostic ignored "-Wstack-usage="
ModemInterpreter::ModemInterpreter(SerialDriver &newDriverPtr,
        SerialDriver &notifyChannel, IoPin *resetPin, uint8_t targetModulID,
        IDiscoveryManager *discoveryManager, ITimeOutWatcher *watcher,
        IoCommandManager &ioCommandManager, AssignmentFlow &assignFlowRef,
        LEDSignalFlow &ledFlowReff, JumpMapper &aliveJumpMapper) :
        modemInterface { newDriverPtr },
        watcher_ { watcher }, ioCmdManager_ { ioCommandManager },
        assignFlowRef_(assignFlowRef), ledFlowReff_ {ledFlowReff },
        aliveJumpMapper_ { aliveJumpMapper } {
    discMangerPtr = discoveryManager;
    modemControll = new ModemController(resetPin, &notifyChannel, assignFlowRef_);
    modulID = targetModulID;
#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
    modemBootloader = new ModemBootLoader(&modemInterface,
            BootCommandManager::bps115200, BootCommandManager::bps115200);
    notUploading = true;
#endif
    networkMemberCount = 1;
    aktivNetMemberCount = 0;
    lastMessageSourceId = 0;
    lastTargetRoutingPeer = 0;
    networkSearchDurationSeconds = 10;
    // networkMemberList.add(NetworkMember(0,1));
    interpretState = idle;
    switchMatrixRowIndex = 0;
    termFilterPtr_ = new TermContextFilter(&notifyChannel);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    transmitter = new ModemTransmitter(&modemInterface, termFilterPtr_);

    CRC32::crc32_init();
}

ModemInterpreter::~ModemInterpreter() = default;

#pragma GCC diagnostic ignored "-Wstack-usage="
void ModemInterpreter::handelRequests() {
    char *modemAnswer = NULL;
    PlcCommand *requestPtr = NULL;
    static PlcCommand cmd;
    modemAnswer = modemInterface.get_buffer();
    if (modemAnswer) {
        cmd = PlcCommand(modemAnswer);
        requestPtr = &cmd;
    }
    handelPLCRequests(requestPtr);
}

void ModemInterpreter::handelBootloading() {
    char *modemAnswer = NULL;
    modemAnswer = modemInterface.get_buffer();
    loadModemFirmenware(modemAnswer);
}

void ModemInterpreter::distinguishPlcCommandAndInformController(
        const PlcCommand &cmd) {
    if (cmd.getChannel() == channel0) {
        if (cmd.getAccesType() == confirm) {
            distinguishConfirmationAndInformController(cmd);
        } else if (cmd.getAccesType() == indication) {
            distinguishIndicationAndInformController(cmd);
        }
    } else {
        termFilterPtr_->println(TX::cTxtNoChannel1Handling);
    }
}

void ModemInterpreter::distinguishConfirmationAndInformController(
        const PlcCommand &cmd) {
    enum {
        Success = 0, UmacOffset = 6, ControllerOffset = 8, AdpOffset = 11,
    };
    uint8_t position;
    uint8_t cmdID = cmd.getCommandID();
    uint8_t cmdState = cmd.getParameter()[0];  // [0] : Status
    LAYER targetLayer = cmd.getAccesLayer();

//------ special treatment ---// Todo(AME): please move into the appropriate method
    if (targetLayer == adpLayer && cmdID == PATH_DISCOVERY) {
        if (ModemController::isRemotePathReplyAwait()) {
            // ModemController::triggerSendDataWithLengthToNetIPAndRoutingMode(cmd.getParameter(),
            //                    length, netIP, false);
        }
        interpretPathDiscoveryParameter(cmd.getParameter());
    } else if (targetLayer == g3Controller && cmdID == SYSTEM_VERSION_GET) {
        position = RequestFactory::getConfirmPosition(targetLayer, cmdID);
        modemControll->setConfirmationPosition(position);
        ModemController::setInitFinish(true);
    }
//--------/

    if (cmdState == Success) {
        actOnConfirmationSuccess(targetLayer, cmdID, cmd);
    } else {
        if (targetLayer == adpLayer && cmdID == DATA) {
            dataStatusCode = cmdState;
            dataStatusSet = true;
            dataStatusSet = true;
        }
        actOnConfirmationFailur(targetLayer, cmdID, cmdState, cmd);
    }
}

void ModemInterpreter::actOnConfirmationSuccess(LAYER targetLayer,
        uint8_t cmdID, const PlcCommand &cmd) {
    uint8_t position = RequestFactory::getConfirmPosition(targetLayer, cmdID);
    modemControll->setConfirmationPosition(position);
    if (targetLayer == g3Controller) {
        switch (cmdID) {
        case INIT:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)) {
                termFilterPtr_->println(
                    ProgramConfig::isCoordinator() ?
                    "|----------|\r|#" : "|-------|\r|#");
            } else {
                termFilterPtr_->println(TX::dTxtPreInitOK);
            }

            break;
        case SETCONFIG:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)) {
                termFilterPtr_->putchar('#');
            } else {
                termFilterPtr_->println(TX::dTxtBandPlanSelect);
            }
            break;
        }
    } else if (targetLayer == umacLayer) {
        // using namespace PLC_CommandTypeSpecification;
        switch (cmdID) {
        case UMAC_GET:
            StorageAdministrator::storePlcGetConfirmationFromLayer(cmd, umacLayer);
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCData)) {
                termFilterPtr_->println(StorageAdministrator::getStatusInfo());
            }

            if (interpretState == searchFCEntry
                    && uint8Operations::sum2BytesFromLSB(cmd.getParameter() + 1)
                            == macDeviceTable) {
                deviceTableCurrentValue = uint8Operations::sum2BytesFromLSB(
                        cmd.getParameter() + 5);
                interpretState = evaluateFCResponse;
            }
            break;
        }
    } else if (targetLayer == adpLayer) {
        switch (cmdID) {
        case DATA:
            dataStatusCode = 0;
            break;
        case RESET:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)) {
                termFilterPtr_->putchar('#');
            } else {
                termFilterPtr_->println(TX::dTxtADPResetOK);
            }
            break;
        case NETWORK_START:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)
                    && ModemController::getControllerState()
                            == ModemController::init) {
                termFilterPtr_->putchar('#');
            } else {
                termFilterPtr_->println(TX::dTxtNetStart);
            }
            break;
        case DISCOVERY:
            modemControll->storePANDiscriptorContainerFromDiscoveryConfirmation(cmd);
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterRoutingInfo)) {
                for (uint8_t containerIndex = 0;
                        containerIndex < ModemController::maxPANDescriptorSets;
                        containerIndex++) {
                  ModemController::PANDescriptorContainerToString(
                      Global2::OutBuff, Global2::outBuffMaxLength, containerIndex);
                    if (Global2::OutBuff[0] != 0) {
                        termFilterPtr_->println(Global2::OutBuff);
                    } else {
                        break;
                    }
                }
                termFilterPtr_->println(TX::dTxtNetScanFinish);
            }
            discMangerPtr->storeData('M', NULL, 0);
            break;
        case NETWORK_JOIN: {
            termFilterPtr_->println(TX::dTxtJoinOK);
            modemControll->activateCoordinatorWithJoinConfirmation();
            uint16_t tmp_ip = uint8Operations::sum2BytesFromLSB(
                    cmd.getParameter() + 1);
            uint16_t tmp_pan = uint8Operations::sum2BytesFromLSB(
                    cmd.getParameter() + 3);
          modemControll->updateAddressInfoWithJoinParameter(tmp_ip, tmp_pan);
            NetworkMember *selfNetMemberPtr =
                    ModemController::getNetMemberPtrByIndex(1);
            selfNetMemberPtr->setOutgoingRoute(NetworkMember::plcConfirmed);
            ModemController::setConnectedToCoordinator(true);
            break;
        }
        case SET:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)
                    && ModemController::getControllerState()
                            == ModemController::init) {
                termFilterPtr_->putchar('#');
            } else {
                if (termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCData)) {
                    termFilterPtr_->println(TX::dTxtADPSetOK);
                }
            }
            break;
        case GET:
            StorageAdministrator::storePlcGetConfirmationFromLayer(cmd, adpLayer);
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCData)) {
                termFilterPtr_->println(StorageAdministrator::getStatusInfo());
            }
            break;
        case ROUTE_DISCOVERY:  // 18.06.24 out dated
            // evaluateAndAnswerRouteDiscoveryResult(cmd);
            break;
        }
    } else if (targetLayer == eapLayer) {
        switch (cmdID) {
        case EAP_RESET:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)
                    && ModemController::getControllerState()
                            == ModemController::init) {
                termFilterPtr_->putchar('#');
            } else {
                termFilterPtr_->println(TX::dTxtEAPResetOK);
            }
            break;
        case EAP_START:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)
                    && ModemController::getControllerState() == ModemController::init) {
                termFilterPtr_->print("#|\r ");
                termFilterPtr_->println(TX::InitializationSuccessfulMsg);
            } else {
                termFilterPtr_->println(TX::dTxtEAPMStart);
            }
            break;
        case EAP_SET:
            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)
                    && ModemController::getControllerState() == ModemController::init) {
                termFilterPtr_->putchar('#');
            } else {
                termFilterPtr_->println(TX::dTxtEAPSetOK);
            }
            break;
        case EAP_SETCLIENTINFO:
            termFilterPtr_->println(TX::dTxtClientInfoOK);
            break;
        }
    }
}

void ModemInterpreter::actOnConfirmationFailur(LAYER targetLayer, uint8_t cmdID,
        uint8_t cmdState, __attribute__((unused)) const PlcCommand &cmd) {
    if (targetLayer == g3Controller) {
        switch (cmdID) {
        case INIT:
            break;
        case SETCONFIG:
            break;
        }
    } else if (targetLayer == umacLayer) {
        // using namespace PLC_CommandTypeSpecification;
        switch (cmdID) {
        case UMAC_GET:
            break;
        }
    } else if (targetLayer == adpLayer) {
        switch (cmdID) {
        case DATA:
            break;
        case DISCOVERY:
            discMangerPtr->storeData('m', NULL, 0);  // Null: data stored already in PanDescriptorContainer
            break;
        }
        namespace plcSpec = PLC_CommandTypeSpecification;
        switch (cmdState) {
        case ADP_STATUS_SUCCESS:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_SUCCESS");
            break;
        case ADP_STATUS_INVALID_PARAMETER:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_INVALID_PARAMETER");
            break;
        case ADP_STATUS_NO_BEACON:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_NO_BEACON");
            break;
        case ADP_STATUS_UNSUPPORTED_ATTRIBUTE:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_UNSUPPORTED_ATTRIBUTE");
            break;
        case ADP_STATUS_INVALID_INDEX:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_INVALID_INDEX");
            break;
        case ADP_STATUS_READ_ONLY:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_READ_ONLY");
            break;
        case ADP_STATUS_INVALID_REQUEST:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_INVALID_REQUEST");
            break;
        case ADP_STATUS_INVALID_IPV6_FRAME:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_INVALID_IPV6_FRAME");
            break;
        case ADP_STATUS_ROUTE_ERROR:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_ROUTE_ERROR");
            break;
        case STATUS_NOT_PERMITTED:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "STATUS_NOT_PERMITTED");
            break;
        case ADP_STATUS_TIMEOUT:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_TIMEOUT");
            break;
        case ADP_STATUS_ALREADY_IN_PROGRESS:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_ALREADY_IN_PROGRESS");
            break;
        case ADP_STATUS_INCOMPLETE_PATH:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_INCOMPLETE_PATH");
            break;
        case ADP_STATUS_REQ_QUEUE_FULL:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_REQ_QUEUE_FULL");
            break;
        case ADP_STATUS_FAILED:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_FAILED");
            break;
        case ADP_STATUS_CONFIG_ERROR:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_CONFIG_ERROR");
            break;
        case ADP_STATUS_INSUFFICIENT_MEMSIZE:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_INSUFFICIENT_MEMSIZE");
            break;
        case ADP_STATUS_IF_NO_RESPONSE:
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::FailureStatusCode),
                    "ADP_STATUS_IF_NO_RESPONSE");
            break;
        }
        termFilterPtr_->println(Global2::OutBuff);
    } else if (targetLayer == eapLayer) {
        switch (cmdID) {
        case EAP_RESET:
            break;
        case EAP_START:
            break;
        case EAP_SET:
            break;
        case EAP_SETCLIENTINFO:
            break;
        }
    }
}

void ModemInterpreter::interpretPathDiscoveryParameter(const uint8_t *discoveryParamPtr) {
    if (discoveryParamPtr[2] == 0) {
        termFilterPtr_->printPln(TX::getText(TX::cTxtDiscoverMetric), discoveryParamPtr[3]);
        termFilterPtr_->printPln(TX::getText(TX::cTxtInteger3Digits), ModemController::getNetworkIPAddress());
        uint16_t offset = 2;
        for (int i = 0; i < discoveryParamPtr[6]; i++) {
            offset = (uint16_t) (offset + 4);
            double lqi = ModemController::calculateLinkQualityinDB(discoveryParamPtr[offset + 4]);
            termFilterPtr_->printPln(TX::getText(TX::cTxtDoubleDb), lqi);
            termFilterPtr_->printPln(TX::getText(TX::cTxtInteger3Digits),
                    (discoveryParamPtr[offset + 1] << 8) + discoveryParamPtr[offset + 2]);
            if ((i > 0) && ((i % 4) == 0)) {
                termFilterPtr_->println("");
            }
        }

        termFilterPtr_->println(TX::dTxtPDiscLegend);
        termFilterPtr_->println(TX::dTxtPDiscEnd);
    }
}

void ModemInterpreter::distinguishIndicationAndInformController(
        const PlcCommand &cmd) {
    LAYER targetLayer = cmd.getAccesLayer();
    uint8_t cmdID = cmd.getCommandID();
    if (targetLayer == g3Controller) {
        switch (cmdID) {
        case DUMP:
            break;
        case EVENT:
            break;
        }
    } else if (targetLayer == umacLayer) {
    } else if (targetLayer == adpLayer) {
        switch (cmdID) {
        case DATA:
            storeSrcAdressFromDataIndication(cmd);
            interpretIncommingMessage(cmd);
            if (!ModemController::isDisplayMessageSupressed()) {
                bool showData = termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCData);
                if (showData) {
                    displayMessageSrcHeader(
                        PlcCommand::getSrcIPFromDataCMD(cmd));
                }
                if (termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCText)) {
                    displayMessageOnTerminal();
                }
                if (showData) {
                    displaySignalQualityOnTerminal(cmd);
                }
            }
            break;
        case NETWORK_LEAVE:
            break;
        case ADPM_LBP:
            break;
        case NETWORK_STATUS:
            if (cmd.getParameter()[0] == 0xDB) {  // R_G3MAC_STATUS_COUNTER_ERROR
                termFilterPtr_->println("R_G3MAC_STATUS_COUNTER_ERROR");
                termFilterPtr_->println(TX::getText(TX::SearchingForFCEntry));
                deviceTableSearchValue = uint8Operations::sum2BytesFromLSB(
                        cmd.getParameter() + 4);
                ModemController::triggerSinglePlcRequestWithArgs(
                        PLC_CommandTypeSpecification::getMacAttribute, 3,
                        channel0, macDeviceTable, 0);
                interpretState = searchFCEntry;
            }
            break;
        case NETWORK_JOIN:
            termFilterPtr_->println(TX::dTxtJoinOK);
            break;
        case BUFFER:
            break;
        case KEY_STATE:
            break;
        case ROUTE_ERROR:
            break;
        case ADP_KEY:
            break;
        }
    } else if (targetLayer == eapLayer) {
        switch (cmdID) {
        case EAP_JOIN: {
            ModemController::setAliveCheckEnabled(false);
            if (modemControll->activateMemberFromJoinIndication(cmd)) {
                uint16_t joinedMemberIdx =
                        modemControll->getLastJoinedMemberIndex();
                if (joinedMemberIdx > 0
                        && joinedMemberIdx
                                <= ModemController::networkMemberMax) {
                    NetworkMember *joinedMember =
                            modemControll->getNetMemberPtrByIndex(
                                    joinedMemberIdx);
                    bool isNewMemberJoined = modemControll->isNewMemberJoined();
                    transmitter->announceJoinWithNetMemberAndAwarenessState(
                            joinedMember, isNewMemberJoined);
                    if (isNewMemberJoined) {
                        ProgramConfig::getPlcModemPtr()->getAssigntmentFlowRef().refreshRoutingDelay();
                    }
                }
            }
            break;
        }

        case EAP_LEAVE:
            break;
        case EAP_NEWDEVICE:
            // if (modemControll->getControllerState() == ModemController::idle) {
            termFilterPtr_->println(TX::dTxtNewDeviceDetect);
            ModemController::setAliveCheckEnabled(false);
            modemControll->triggerNewDeviceAuthentification(cmd, channel0);

            //  } else {
            //     ModemInterpreter::modemCrierPtr->println(
            //     "Modem Controller ist Beschaeftigt!");
            //  }

            break;
        }
    }
}

void ModemInterpreter::storeSrcAdressFromDataIndication(
        const PlcCommand &confirmation) {
    enum {
        ipv6SrcShortAdressOffset = 24
    };
    const uint8_t *paramPtr = confirmation.getParameter();
    lastDataIndicationSrcIP_ = uint8Operations::sum2BytesFromLSB(
            paramPtr + ipv6SrcShortAdressOffset);
}

void ModemInterpreter::handelPLCRequests(const PlcCommand *requestPtr) {
    static PlcCommand *answer = NULL;
    bool done;
    if (requestPtr) {
        if (TermContextFilter::isNotFiltered(TermContextFilter::filterPLCInput)) {
            termFilterPtr_->println(TX::ShowModemAnswer);
            char *requestString = requestPtr->toCommandStringHead();
            termFilterPtr_->println(requestString);
            requestString = requestPtr->toCommandStringData(0, &done);
            termFilterPtr_->println(requestString);
            if (!done) {
                requestString = requestPtr->toCommandStringData(1, &done);
                termFilterPtr_->println(requestString);
            }
            termFilterPtr_->println("");
        }
        distinguishPlcCommandAndInformController(*requestPtr);
    }
    modemControll->incrementProcessStateIfConfirmed();
    answer = modemControll->executeProcessStateAndGetAnswer();
    if (answer) {
        char *msg = answer->toRawString2();
        modemInterface.printRawFast(msg, answer->getStringLength());
        if (TermContextFilter::isNotFiltered(TermContextFilter::filterPLCOutput)) {
            termFilterPtr_->println(TX::ShowModemRequest);
            char *requestString = answer->toCommandStringHead();
            termFilterPtr_->println(requestString);
            requestString = answer->toCommandStringData(0, &done);
            termFilterPtr_->println(requestString);
            if (!done) {
                requestString = answer->toCommandStringData(1, &done);
                termFilterPtr_->println(requestString);
                termFilterPtr_->println("");
            }
        }
    }
}

#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
void ModemInterpreter::loadFirmenwareToModem(char *modemAnswer,
        const PlcCommand *requestPtr) {
    static bool isErrorNotNotified = true;
    if (modemBootloader->isFirmenwareInStorage()) {
        proofModemStatusAndLoadSoftware(modemAnswer, requestPtr);
        softwareNotInStorage = false;
        ledFlowReff_.setLedMode(LEDSignalFlow::noSoftware);
    } else if (isErrorNotNotified) {
        termFilterPtr_->println(TX::dTxtSoftNotInMem);
        softwareNotInStorage = true;
        // isErrorNotNotified = false;
    }
}

void ModemInterpreter::proofModemStatusAndLoadSoftware(char *modemAnswer,
        __attribute__((unused)) const PlcCommand *requestPtr) {
    if (modemBootloader->isFirmenwareStatusUnkown()) {
        ModemController::setFirmenwareLoaded(false);
        // proofOnceFirmenwareLoaded(requestPtr);
        loadModemFirmenware(modemAnswer);
    } else {
        ModemController::setFirmenwareLoaded(true);
        MainLoop::stop();
    }
}

void ModemInterpreter::loadModemFirmenware(char *modemBootAnswer) {
    static BootCommandManager::CommandID commandID = BootCommandManager::ERROR;
    if (!ModemController::isFirmenwareLoaded()) {
        if (modemBootAnswer) {
            commandID = (BootCommandManager::CommandID) modemBootAnswer[0];
            displayBootSteps(commandID);
            modemBootloader->reactOnRequest(commandID);
            if (!modemBootloader->isFirmenwareStatusUnkown()) {
                ModemController::setFirmenwareLoaded(true);
                MainLoop::stop();
                return;
            }
        }
        modemBootloader->updateTransmissionBlockPtr();
    }
}

void ModemInterpreter::displayBootSteps(
        BootCommandManager::CommandID commandID) {
//  static char stepMessage[36];
    switch (commandID) {
    case BootCommandManager::SegmentRequest0:
        termFilterPtr_->println(TX::cTxtModemFwLoading);
        __attribute__((fallthrough));
        // no break
    case BootCommandManager::SegmentRequest1:
    case BootCommandManager::SegmentRequest2:
    case BootCommandManager::SegmentRequest3:
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtModemFwSegRequest),
                static_cast<char>(commandID - 0x50));
        break;
    case BootCommandManager::BaudrateChangeRequest:
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtBaudChangeRequest));
        break;
    case BootCommandManager::FirmenwarerTansmissionSucess:
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::dTxtUploadOK));
        break;
    case BootCommandManager::BaudrateSetAccept:
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtBaudChangeAccept));
        break;
    default:
        break;
        // case BootCommandManager::BaudrateSet:
        // case BootCommandManager::BaudrateUsedSucess:
        // case BootCommandManager::OPERATE:
        // case BootCommandManager::ERROR:
        //      sprintf(Global2::OutBuff, "Baudrate gesetzt.");
    }
    termFilterPtr_->println(Global2::OutBuff);
}
#endif  // IF IS_FIRMWARE_DOWNLOAD_ENABLED >= 1

void ModemInterpreter::displayMessageSrcHeader(uint16_t srcIP) {
    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCText)) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), TX::getText(TX::SourceIP), srcIP);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "\n%80s", tmp);
        termFilterPtr_->println(Global2::OutBuff);
    }
}

void ModemInterpreter::displayMessageOnTerminal() {
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%80s", TX::getText(TX::cTxtMessage));
    termFilterPtr_->println(Global2::OutBuff);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%80s", Global2::InBuff);
    termFilterPtr_->println(Global2::OutBuff);
}

void ModemInterpreter::displaySignalQualityOnTerminal(
        const PlcCommand &requestPtr) {
    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterPLCText)) {
        char signalStrengthMessage[32];
        const uint8_t *pramPtr = requestPtr.getParameter();
        uint16_t nsduLength = (uint16_t) ((pramPtr[0] << 8) + (pramPtr[1]));
        float signalStrength = ModemController::calculateLinkQualityinDB(
                pramPtr[nsduLength]);
        snprintf(signalStrengthMessage, Global2::outBuffMaxLength,
                TX::getText(TX::cTxtSignalQuality), signalStrength);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%80s\n",
                signalStrengthMessage);
        termFilterPtr_->println(Global2::OutBuff);
    }
}

void ModemInterpreter::interpretIncommingMessage(const PlcCommand &requestPtr) {
    const uint8_t *messagePtr = ModemController::getMessagePtrFromPlcCommand(requestPtr);
    memcpy(Global2::InBuff, messagePtr, sizeof(Global2::InBuff));
#if ALIVE_CHECK_LEVEL == 1
    static int aliveResponseCnt = 0;    // counter for alive requests
#endif

    char *commandStartPtr = strstr(Global2::InBuff, commandPrefix);
    if (commandStartPtr) {
        char *commandLetter = commandStartPtr + 2;

        switch (*commandLetter) {
        case AliveRequest:  // handle alive request of coordinator to peer
        {
//          if (ProgramConfig::isCoordinator()) {
//              uint16_t recivingIP = atoi(++commandLetter);
//              if (recivingIP == ModemController::getAlivePrevGroup()) {
//                  uint16_t act   = ModemController::getAliveCheckActLoops();
//                  char* monitorLog = ModemController::monitorAliveStatusAndGetLog();
//                  termFilterPtr_->println(monitorLog);
//                  TimeMeasurement::resetJobTime(*ModemController::getAliveTimeOutJob());
//                  ModemController::setAliveCallOutCounter(0);
//                  if (act != 0) {
//                      snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::LoopsMsg), act);
//                      termFilterPtr_->println(Global2::OutBuff);
//                      ModemController::setAliveCheckActLoops(--act);
//                      ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
//                              reinterpret_cast<uint8_t*>("::A"), 3, ModemController::getAliveNextGroupe());
//                  } else {
//                      TimeMeasurement::measureJob& job = *ProgramConfig::getAliveTimeJobPtr();
//                      snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
//                              "alive request was handled within [%d]ms", TimeMeasurement::getJobDeltat(job) );
//                      termFilterPtr_->println(Global2::OutBuff);
//                      TimeMeasurement::unregisterTimedJob(job);
//                  }
//                  // char tmp[20];
//                  // sprintf(Global2::OutBuff,"Schleifentest Alive: %s",
//                  //         TimeMeasurement::toString(ProgramConfig::getAliveTimeJobIdx(),
//                  //         tmp, TimeMeasurement::TimeHHMMSSms));
//                  // ModemInterpreter::modemCrierPtr->println(Global2::OutBuff);
//              }
//          }
            // ModemController::setSupressDisplayMessage(
            //                  transmitter->getV e r b o s e Mode() > ModemTransmitter::v e r b o s e _Detailed);
            // sprintf(Global2::OutBuff,"%s%c%03d",ModemInterpreter::commandPrefix,AliveReply,ModemController::getMacShortAddress());
            // ModemController::triggerSendDataWithMessageToNetMemberAndRoutingMode(Global2::OutBuff, 0, false);
            break;
        }

        case errorAliveNoResponseRequest: {
            // Reaction on "E"-alarm message from peer
            unsigned int errorReporterIp, tripID;
            unsigned int cnt = sscanf(commandLetter + 1, ":%u:%u",
                    &errorReporterIp, &tripID);
            if (cnt == 2) {
                bool isRequestValid =
                        aliveJumpMapper_.isErrorReporterRequestValid(
                                (uint16_t) errorReporterIp, (uint8_t) tripID);
                if (ProgramConfig::isCoordinator()) {
                    if (!ForwardingFlow::isRoundTripFinished()) {
                        if (isRequestValid) {
                            aliveJumpMapper_.reactOnAliveNoResponseRequest(
                                    (uint16_t) errorReporterIp,
                                    (uint8_t) tripID);
                        } else {
                            if (aliveJumpMapper_.isReporterLastInRoute()) {
                                BGroupForwarding::handleLastAckReply(-1);
                            }
                            if (termFilterPtr_->isNotFiltered(TermContextFilter::filterAliveJumpInfo)) {
                                aliveJumpMapper_.informUserAboutJumpInvalidity();
                            }
                        }
                    }
                  aliveJumpMapper_.reply2NoResponseRequestReporter(
                      (uint16_t) errorReporterIp);  // acknowledge "e"
                }
            }
            break;
        }
        case errorAliveNoResponseReply: {
            unsigned int errorReporterIp;
            unsigned int cnt = sscanf(commandLetter + 1, ":%u",
                    &errorReporterIp);
            if (cnt == 1) {
                if (errorReporterIp == ModemController::getNetworkIPAddress()) {
                    watcher_->stopAndReset(ProgramConfig::aliveNoResponsReportTOutID);
                    watcher_->stopAndReset(ProgramConfig::rAliveTOutID);
                    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ReportLossOfMember));
                    termFilterPtr_->println(Global2::OutBuff);
                }
            }
            break;
        }
        case AliveStopRequest: {
            unsigned int dummy;
            unsigned int cnt = sscanf(commandLetter + 1, "%u", &dummy);
            if (cnt == 1) {
              ForwardingFlow::setRoundTripFinished(true);
              ForwardingFlow::setRoundTripStopDelay(2);
            }
            break;
        }
        case AliveLastAckReply: {
            int tripID;
            int cnt = sscanf(commandLetter + 1, ":%03u", &tripID);
            if (cnt == 1) {
                BGroupForwarding::handleLastAckReply(tripID);
            }
            break;
        }
        case TimeoutsGlobalRequest:  // set timeout values within the device via global PLC telegram
            if (strstr(commandLetter + 1, "GlobalTimeout")) {
                unsigned int cnt, id, delay, toAmount, coAmount;
                const char *txt = charOperations::skip_token(commandLetter, ' ');
                cnt = sscanf(txt, "%u %u %u %u", &id, &delay, &toAmount, &coAmount);
                if ((cnt < 4) && (id > 20) && (delay > 1000) && (toAmount > 20) && (coAmount > 10)) {
                } else {
                    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
                    ITimeOutWatcher::timeOutInfo *pToi = pW->findTimeOutWithID((uint8_t) id);
                    if (pToi) {
                        pToi->delay = (uint16_t) delay;
                        pToi->timeOutAmount = (uint8_t) toAmount;
                        pToi->callOutCounter = (uint8_t) coAmount;
                        termFilterPtr_->println(TX::TimeoutValuesSet);
                        break;
                    }
                }
            }
            termFilterPtr_->println(TX::ExternalTimeoutsRejected);
            break;

        case SwitchMatrixRequest: {
            if (isRequestForThisModul(commandLetter)) {
                char *replyMessage = ioCmdManager_.transformSwitchRequest(
                        commandLetter);
                termFilterPtr_->println(replyMessage);
            }
            break;
        }
        case switchMatrixReply: {
            if (isRequestForThisModul(commandLetter)) {
                char *replyMessage = ioCmdManager_.transformSwitchReply(
                        commandLetter);
                termFilterPtr_->println(replyMessage);
            }
            break;
        }
        case SwitchCommandConfirmation: {  // treatment of a successful Switch-Matrix request
            uint8_t responseIP = (uint8_t) (commandLetter[1] - '0');
            SwitchMatrixAdmin::setAckForIndex(responseIP);
            break;
        }
        case PeerLeaveRequest: {
            // leaveNetwork(" ");  outdated
            break;
        }
        case RoutingMessage:  // 18.06.2024 routing out dated
            break;

//      case PathDiscoveryRequest: {  // will not pursued further 15.02.2024
//          commandLetter += 1;
//          uint16_t destID = atoi(commandLetter);
//          ModemController::triggerSinglePlcRequestWithArgs(
//                          PLC_CommandTypeSpecification::pathDiscovery, 3,
//                          channel0,
//                          destID,
//                          0x0F);
//          ModemController::setRemotePathReplyAwait(true);
//          break;
//      }
//      case PathDiscoveryReply:
//          interpretPathDiscoveryParameter((uint8_t*)commandLetter);
//          break;

        case InputRequest: {
            if (isRequestForThisModul(commandLetter)) {
                ioCmdManager_.answerPinStatusWithReplyType(InputReply);
            }
            break;
        }
        case InputReply: {
            if (isRequestForThisModul(commandLetter)) {
                bool isInput = true;
                char *replyMessage = ioCmdManager_.transformInOrOutReply(
                        commandLetter, isInput);
                termFilterPtr_->println(replyMessage);
            }
            break;
        }

        case OutputRequest: {
            if (isRequestForThisModul(commandLetter)) {
                ioCmdManager_.reactOnOutputReqeuest(commandLetter);
                ioCmdManager_.answerPinStatusWithReplyType(outputReply);
            }
            break;
        }
        case outputReply: {
            if (isRequestForThisModul(commandLetter)) {
                bool isInput = false;
                char *replyMessage = ioCmdManager_.transformInOrOutReply(
                        commandLetter, isInput);
                termFilterPtr_->println(replyMessage);
            }
            break;
        }

        case PollRequest: {
            if (isRequestForThisModul(commandLetter)) {
                const char *subCmd = commandLetter + 5;
                ioCmdManager_.answerPollRequest(subCmd);
            }
            break;
        }
        case pollReply: {
            if (isRequestForThisModul(commandLetter)) {
                char *replyMessage = ioCmdManager_.transformPollReply(
                        commandLetter);
                termFilterPtr_->println(replyMessage);
            }
            break;
        }
        case ForceJoinCheckRequest: {
            uint16_t IPAdr;
            uint16_t MacShort;
            uint8_t ignoreNTimes;
            NetworkMember *netListMac, *netListIP;
            SimpleFlashEditor::Parameter_A sys_param;
            int sucessorIP;
            bool isNoMember, isMacEqual, isNotDisconnected;
            commandLetter += 1;
            if (ProgramConfig::isCoordinator()) {
                switch (*commandLetter) {
                case 'R':  // Request
                    termFilterPtr_->println(TX::cTxtForceJoinQuitMsg);
                    MacShort = (uint16_t) atoi(commandLetter + 1);
                    IPAdr = (uint16_t) atoi(commandLetter + 6);
                    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "MacAdr:%d Src-IP:%d", MacShort, IPAdr);
                    termFilterPtr_->println(Global2::OutBuff);
                    // check network member mapping
                    netListMac = ModemController::findMemberWithShortID(MacShort);
                    netListIP = ModemController::getNetMemberPtrByIP(IPAdr);
                    isNoMember = netListMac != NULL;
                    isMacEqual = netListMac == netListIP;
                    isNotDisconnected = netListMac->getLink() != NetworkMember::disconnected;
                    if (isNoMember && isMacEqual && isNotDisconnected) {
                        ignoreNTimes = AutostartFlow::getIgnoreNTimesFJRequests();
                        if (ignoreNTimes == 0) {
                            netListMac->setLink(NetworkMember::connected);
                            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%cA%02d",
                                    ModemInterpreter::commandPrefix,
                                    ModemInterpreter::ForceJoinCheckRequest,
                                    IPAdr);
                            ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                                    reinterpret_cast<uint8_t*>(Global2::OutBuff), 7,
                                    IRoutingManager::BCGlobalIP, true,
                                    TermContextFilter::filterForceJoinInfo);
                        } else {
                            termFilterPtr_->println(TX::ForceJoinCheckIgnore1x);
                            AutostartFlow::setIgnoreNTimesFJRequests(
                                    (uint8_t) (ignoreNTimes - 1));
                        }
                    } else {
                        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                                TX::getText(TX::ForceJoinCheckFailInfo),
                                isNoMember, (unsigned int) netListMac,
                                (unsigned int) netListIP, isNotDisconnected);
                        termFilterPtr_->println(Global2::OutBuff);
                    }
                    interpretState = idle;
                    break;
                case 'A':  // Acknowledge
                    IPAdr = (uint16_t) atoi(commandLetter + 1);
                    netListIP = ModemController::getNetMemberPtrByIP(IPAdr);
                    if (netListIP != NULL) {
                        netListIP->setLink(NetworkMember::connected);
                        setForceJoinState(FJconfirmed);  // DS 12.06.2023 handshake not required any more
                        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                                TX::getText(TX::ForceJoinCheckReceived), IPAdr);
                        termFilterPtr_->println(Global2::OutBuff);
                    }
                    break;
                }
            } else {  // Peer
                switch (*commandLetter) {
                case 'A':  // Acknowledge
                    commandLetter++;
                    sucessorIP = atoi(commandLetter);
                    if (sucessorIP == modemControll->getNetworkIPAddress()) {
                        sys_param = SimpleFlashEditor::getFlashA();
                      modemControll->updateAddressInfoWithJoinParameter(
                          sys_param.ownNetworkAddress,
                          sys_param.targetNetId);
                        setForceJoinState(FJconfirmed);
                    }
                    break;
                case 'C': {  // coordinator request
                    ignoreNTimes = AutostartFlow::getIgnoreNTimesFJRequests();
                    if (ignoreNTimes == 0) {
                        commandLetter++;
                        sucessorIP = atoi(commandLetter);
                        IPAdr =
                            ModemController::getNetMemberPtrByIndex(1)->getNetworkIPAddress();
                        if (sucessorIP == IPAdr) {
                            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%cA%04d",
                                    ModemInterpreter::commandPrefix,
                                    ModemInterpreter::ForceJoinCheckRequest, IPAdr);
                            ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                                    reinterpret_cast<uint8_t*>(Global2::OutBuff), 8,
                                    IRoutingManager::BCGlobalIP, true,
                                    TermContextFilter::filterForceJoinInfo);
                            setForceJoinState(c2pIPReceived);
                        } else {
                            setForceJoinState(c2pBCReceived);
                        }
                    } else {
                        termFilterPtr_->println(TX::ForceJoinCheckIgnore1x);
                        AutostartFlow::setIgnoreNTimesFJRequests((uint8_t) (ignoreNTimes - 1));
                    }
                    break;
                }
                }
            }
            break;
        }

        case SearchRequest: {
            commandLetter += 1;
            if (ProgramConfig::isCoordinator()) {
                if (*commandLetter == 'A' || *commandLetter == 'D'
                        || *commandLetter == 'N') {
                    discMangerPtr->storeData(*commandLetter, messagePtr,
                            requestPtr.getDataMessageLength());
                    assignFlowRef_.processAssignment(
                            AssignmentFlow::refreshOuterTimeOut);
                }
            } else {  // 'R'=request message of coordinator, 'd'=data request.
                if ((*commandLetter == 'R') || (*commandLetter == 'd')) {
                    discMangerPtr->storeData(*commandLetter, messagePtr, 0);
                }
            }
            break;
        }
            // Peer:
        case BroadCastGroupReq: {
            commandLetter += 1;
            enum eErrNo {
                none, fehler
            };
            enum eErrNo errNo = none;
            AssignmentFlow::TupelContext *pTupelContext =
                    AssignmentFlow::getTupelContext(AssignmentFlow::transfer);
            IRoutingManager::MemberIpTupel *pTupel = pTupelContext->pTupelArray;
            unsigned int i, destIP, status, modemRegOffset, aliveMappingOffset, numTupel, nIP, nMemberID;
            uint32_t cnt = sscanf(commandLetter, "%03u:%u:%u:%u:%u", &destIP,
                    &status, &modemRegOffset, &aliveMappingOffset, &numTupel);
            const uint8_t expectedAmount = 5;
            if (cnt == expectedAmount) {
                nIP = ModemController::getNetMemberPtrByIndex(1)->getNetworkIPAddress();
                if (nIP != destIP) {
                    break;
                }
                for (i = 1; i <= expectedAmount; i++) {  // commandLetter+=11;
                    if (*commandLetter) {  // (char *)
                        commandLetter = (char *) charOperations::skip_token(commandLetter, ':');
                    }
                }
                for (i = 0; i < numTupel; i++) {
                    cnt = sscanf(commandLetter, "%03u,%03u", &nMemberID, &nIP);
                    if (cnt == 2 && i < IRoutingManager::maxTupelAmount) {
                        pTupel[i].meberID = (uint8_t) nMemberID;
                        pTupel[i].ip = (uint16_t) nIP;
                        commandLetter += 7;
                        if (i < (numTupel - 1) && *commandLetter++ != ':') {
                            errNo = fehler;
                            break;
                        }
                    } else {
                        errNo = fehler;
                        break;
                    }
                }
            } else {
                errNo = fehler;
            }
            switch (errNo) {
            case none:
                i = assignFlowRef_.getPeerIgnoreAssignRequests();
                if (i == 0) {
                    termFilterPtr_->println(TX::AddressesAssigned);
                    pTupelContext->ofsRegModem = (uint8_t) modemRegOffset;
                    pTupelContext->ofsAliveMap = (uint8_t) aliveMappingOffset;
                    pTupelContext->numTupelEntries = (uint8_t) numTupel;
                    interpretState = triggerBGroupSet;
                } else {
                    termFilterPtr_->println(TX::AddressesIgnored1x);
                    assignFlowRef_.setPeerIgnoreAssignRequests((uint8_t) (i - 1));
                }
                break;
            case fehler:
                termFilterPtr_->println(TX::InterpretationError);
                break;
            }
            break;
        }
            // coordinator:
        case BroadCastGroupReply: {
            int receivingIP;
            ModemController::eBGTransmitMode nMode =
                    (ModemController::eBGTransmitMode) ModemController::getBGtransmitMode();
            switch (nMode) {
            case ModemController::manuell:
                termFilterPtr_->println(TX::GroupAssignmentSuccessful);
                ModemController::setBGtransmitMode(ModemController::bgIdle);
                break;
            case ModemController::processAuto:
                receivingIP = atoi(++commandLetter);
                if (receivingIP == ModemController::getGlobalDestIp()) {
                    AssignmentFlow::handlePeerAssigned((uint16_t) receivingIP);
                    interpretState = waitBGAutoGroupSet;
                }
                break;
            default:
                break;
            }
            // ProgramConfig::getTimeOutWatcherPtr()->clearTimeout(ProgramConfig::rBroadcastGroupTOutID);
            ProgramConfig::getTimeOutWatcherPtr()->stopAndReset(ProgramConfig::assignmentFlowTOutID);
            break;
        }

        case GlobalResetRequest:
            commandLetter += 1;
            if (strstr(commandLetter, "GlobalReset")) {
                triggerOwnResetRequest();
            }
            break;
        case aliveJumpRequest:
            if (isRequestForThisModul(commandLetter)) {
                unsigned int srcIP, destGroupID, roundtripID;
                uint32_t cnt = sscanf(++commandLetter, "%u:%u:%u", &srcIP, &destGroupID, &roundtripID);
                if (cnt == 3) {
                    aliveJumpMapper_.reactOnJumpRequest((uint16_t) destGroupID, (uint16_t) roundtripID);
                }
                interpretState = aliveSendReEntryReply;
            }
            break;
        case aliveJumpReply:
            aliveJumpMapper_.reactOnJumpReply();
            break;
        }
    }
}

void ModemInterpreter::triggerOwnResetRequest() {
    ITimeOutWatcher *pW = ProgramConfig::getTimeOutWatcherPtr();
    pW->clearTimeout(ProgramConfig::simpleShortTOutID);
    ITimeOutWatcher::timeOutInfo *pToi = pW->findTimeOutWithID(ProgramConfig::simpleShortTOutID);
    if (pToi == NULL) {
        ITimeOutWatcher::timeOutInfo toi;
        ITimeOutWatcher::createDefaultTimeOut(&toi,
                                              ProgramConfig::simpleShortTOutID, &handleOwnResetRequest, NULL);
        pW->registerTimeout(toi);
        termFilterPtr_->println(TX::HandleExternalResetRequest);
    }
}

void ModemInterpreter::handleOwnResetRequest() {
    MicrocontrollerConfig::softwareReset();
}

void ModemInterpreter::answerStepwiseOnIncommingMessages() {
    static uint16_t counter = 0;
    // DS- static uint8_t previousTargetRoutingMemberIndex = 0;
    static uint8_t routingindex = 1;
    static uint8_t callOut = 0;
    const char *msg;
    if ((interpretState == idle) && (nextInterpretState != idle)) {
        interpretState = (InterpretState) nextInterpretState;
        nextInterpretState = idle;
    }
    switch (interpretState) {
    case aliveCheckRequest: {
        if (DelayHandler::hasWaitNTimes(4)) {
#if ALIVE_CHECK_LEVEL == 1
          ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(aliveResponsMessage, 0, false);
#endif
            interpretState = idle;
        }
        break;
    }
    case switchMatrixRequest: {
        termFilterPtr_->printPln(TX::getText(TX::cTxtSendK2Peer), lastSwitchMessagePanIP);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%sK%d", commandPrefix,
                 ModemController::getNetworkIPAddress());
        ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(
                Global2::OutBuff, lastSwitchMessagePanIP, false);
        interpretState = switchMatrixSwitch;
        break;
    }
    case switchMatrixSwitch: {
        SwitchMatrixAdmin::executeSwitchWithIndex(switchMatrixRowIndex);
        interpretState = switchMatrixAnswer;
        counter = 0;
        break;
    }
    case switchMatrixAnswer: {
        if (counter < 3) {
            if (hasWaitNTimes(10) || counter == 0) {
                counter++;
                SwitchMatrixAdmin::sendSwitchCommandIndexOverModemForConnectionGroupe(
                        switchMatrixRowIndex, this);  // TODO(AME) test and implement again
            }
        } else {
            interpretState = idle;
            counter = 0;
        }
    }
        break;
    case repeatConnectionAnswer: {
        if (counter < 3) {
            counter++;
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%sD", commandPrefix);
            connected = true;
            // ModemController::triggerMessageToNetMemberWithRoutingMode(Global2::OutBuff, 0, true);
        } else {
            interpretState = idle;
            counter = 0;
        }
        break;
    }
    case announceUpdate: {
//      uint8_t lastMemberIndex = networkMemberCount-1;
        // DS- const char routingFinished []= "--------Route Update finished!--------";
        counter = 0;
//      if (lastMemberIndex==0 && aktivNetMemberCount >1){
//          lastMemberIndex = networkMemberMax;
//      } else if (aktivNetMemberCount ==1) {
//          ModemInterpreter::modemCrierPtr->println(routingFinished);
//          peerRoutingProcess = false;
//          interpretState = idle;
//          break;
//      }

//      if((targetRouting.MemberIndex + 1) <= networkMemberCount){
//          routeAllAktivNetMembersToNetID( targetRouting.MemberIndex );
//      } else {
//          ModemInterpreter::modemCrierPtr->println(routingFinished);
//          peerRoutingProcess = false;
//          targetRouting.MemberIndex = 1;
//          interpretState = idle;
//          break;
//      } else if(networkMemberList[targetRouting.MemberIndex].isAktiv()) {
//          requestRoutingFromSourceToTargetPeer(targetRouting.MemberIndex,lastMemberIndex);
//      }
        break;
    }
    case waitAnnounceAnswer: {
        if (counter++ > 250) {
            if (callOut++ >= 6) {
                callOut = 0;
                counter = 0;
                // targetRouting.UpdateIndex++;
            }
            interpretState = announceUpdate;
        }
        break;
    }

//   case startRouteUpdate: {
//      bool isRepeatingSameRoutingRequest =  previousTargetRoutingMemberIndex == targetRouting.MemberIndex;
//      if (isRepeatingSameRoutingRequest && networkMemberList[targetRouting.MemberIndex].isAktiv()) {
//          //uint16_t panAdress = uint8Operations::sum2BytesFromLSB( G3Channel0->getPanAdress());
//          ModemInterpreter::modemCrierPtr->println("Route wurde bereits gefunden!");
//          sprintf(Global2::OutBuff,"%sW%d>>%d",commandPrefix,panAdress,targetRouting.MemberIndex);
//          // ModemController::triggerMessageToNetMemberWithRoutingMode(Global2::OutBuff,
//                  networkMemberList[targetRouting.MemberIndex],false);
//// #ifdef DEBUG
////        if(isRepeatingSameRoutingRequest){
////            sprintf(Global2::OutBuff,
///                 " Nachricht [ V%d>>%d ] gesendet zum Koordinator mit Routing",panAdress,targetRouting.MemberIndex);
////            isV e r b o s e Mode = true;
////        } else {
////            isV e r b o s e Mode = false;
////            sprintf(Global2::OutBuff,
///                 " Nachricht [ V%d>>%d ] gesendet zum Koordinator",panAdress,targetRouting.MemberIndex);
////        }
////        ModemInterpreter::modemCrierPtr->println(Global2::OutBuff);
//// #endif
//          interpretState = idle;
//      } else {
//          previousTargetRoutingMemberIndex = targetRouting.MemberIndex;
//          G3Channel0->doRouteDiscovery(targetRouting.MemberIndex);
//          interpretState = waitRouteUpdate;
//      }
//      break;
//  }
//  case waitRouteUpdate: {
//      if (lastRoutingState == routeSuccess) {
//          if(counter++ > 20){
//              uint16_t panAdress = uint8Operations::sum2BytesFromLSB( G3Channel0->getPanAdress());
//              sprintf(Global2::OutBuff,"%sW%d>>%d",commandPrefix,panAdress,targetRouting.MemberIndex);
//              ModemController::sendMessageToMember(
//                                  Global2::OutBuff, networkMemberList[targetRouting.MemberIndex],false);
//              peerRoutingProcess = false;
//              lastRoutingState = routeIdle;
//              interpretState = idle;
//              // counter = 0;
//          } else if(counter == 1) {
//              uint16_t panAdress = uint8Operations::sum2BytesFromLSB( G3Channel0->getPanAdress());
//              sprintf(Global2::OutBuff,"%sW%d>>%d",commandPrefix,panAdress,targetRouting.MemberIndex);
//              networkMemberList[targetRouting.MemberIndex].setIsAktiv(true);
//              ModemController::triggerMessageToNetMemberWithRoutingMode(
//                                  Global2::OutBuff, targetRouting.MemberIndex,false);
//              ModemInterpreter::modemCrierPtr->printFormatMessageWithNumber(
//                                  "PAN-ID [ %04d ] wurde aktiviert!", targetRouting.MemberIndex);
//              aktivNetMemberCount++;
//              peerRoutingProcess = false;
//              lastRoutingState = routeIdle;
//              interpretState = idle;
//                counter = 0;
//         }
//      } else if ((lastRoutingState == routeFailure)
//              || (lastRoutingState == routeTimeout)) {
//          // DS- previousTargetRoutingMemberIndex = 0;
//      }
//      break;
//  }
//  case waitTillNextRoutingCall: {
//      static int waitTime = 0;
//      if(waitTime++ > 20){
//          waitTime = 0;
//      callOut = 0;
//      interpretState = announceUpdate;
//      }
//      break;
//  }

    // ToDo(TH): check if obsolete
    case updateMemberRouting: {
        if (routingindex <= aktivNetMemberCount) {
            lastRoutingState = routeIdle;
            termFilterPtr_->printPln(TX::getText(TX::cTxtStartRouteMember), routingindex);
            // G3Channel0->doRouteDiscovery(routingindex);
            interpretState = waitMemberRouting;
            counter = 0;
        } else {
            routingindex = 1;
            interpretState = idle;
            termFilterPtr_->println(TX::cTxtMemberRoutingEnd);
        }

        break;
    }

    // ToDo(TH): check if obsolete
    case waitMemberRouting: {
        static uint8_t callOutRouting = 0;
        if (callOutRouting > 3) {
            interpretState = idle;
            termFilterPtr_->println(TX::cTxtRoutingAborted);
        }
        if (counter++ > 1000) {
            callOutRouting++;
            interpretState = updateMemberRouting;
            termFilterPtr_->println(TX::cTxtMemberRouteTimeOut);
        }
        if (lastRoutingState == routeSuccess) {
            routingindex++;
            interpretState = updateMemberRouting;
        } else if ((lastRoutingState == routeFailure)
                || (lastRoutingState == routeTimeout)) {
            callOutRouting++;
            interpretState = updateMemberRouting;
        }
        break;
    }
    case showMembers: {
        ModemController::handleDisplayState();
        break;
    }
    // case proofRoutingTimeout:  // 18.06.2024 routing out dated
    //    break;

    case proofForceJoinVerification: {
        if (dataStatusSet) {
            if (dataStatusCode != 0) {
                setForceJoinState(FJerror);
            }
            interpretState = idle;
        }
        break;
    }
    case proofDiscoveryTimeout:  // see AssignmentFlow::doRouteDiscovery
    case idle:
        break;
    case updateCIFlashStart:
        routingindex = 1;
        interpretState = updateCIFlashNext;
        __attribute__((fallthrough));
        // no break;
    case updateCIFlashNext:
        if (routingindex <= ModemController::networkMemberMax) {  // routingindex <= aktivNetMemberCount
            ModemController::updateStorageClientInfoByIdx(routingindex++);
            if (!ProgramConfig::isCoordinator()) {
                routingindex = 1;
                interpretState = idle;
            }
        } else {
            routingindex = 1;
            interpretState = idle;
        }
        break;
    case triggerBGroupSet: {
        // registering only in memory, not in the plc modem
        AssignmentFlow::TupelContext *pTupelContext =
                AssignmentFlow::getTupelContext(AssignmentFlow::transfer);
        IRoutingManager::MemberIpTupel *pTupel = pTupelContext->pTupelArray;
        assignFlowRef_.addNeighbourTupels(pTupelContext->ofsRegModem,
                pTupelContext->ofsAliveMap, pTupelContext->numTupelEntries,
                pTupel);
        ledFlowReff_.setLedMode(LEDSignalFlow::aliveRunning);
        interpretState = waitBGroupSet;
        break;
    }
    case waitBGroupSet:
        // registering only in memory, not in the plc modem
        // if (ModemController::getControllerState() == ModemController::idle) {
            if (ProgramConfig::isCoordinator()) {
                assignFlowRef_.processAssignment(AssignmentFlow::plcMsg);
            } else {
                // acknowledge to coordinator
                uint16_t ipAdr = ModemController::getNetMemberPtrByIndex(1)->getNetworkIPAddress();
                snprintf(Global2::InBuff, Global2::inBuffMaxLength, "%s%c%03d",
                        ModemInterpreter::commandPrefix,
                        ModemInterpreter::BroadCastGroupReply, ipAdr);
                ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(
                        Global2::InBuff, 0, false);
            }
            interpretState = idle;
        // }
        break;
    case searchFCEntry:
        break;
    case evaluateFCResponse:
        static uint8_t index = 0;
        if (deviceTableCurrentValue == deviceTableSearchValue) {
            ModemController::triggerSinglePlcRequestWithArgs(
                    RequestFactory::createUserSetRequest(
                            ClearDeviceFrameCounter, 3, index,
                            deviceTableSearchValue));
            index = 0;
            deviceTableCurrentValue = 0;
            deviceTableSearchValue = 0;
            termFilterPtr_->println(TX::FrameCounterDeletionFinished);
            interpretState = idle;
        } else if (deviceTableCurrentValue == 0
                && index <= ModemController::networkMemberMax) {
            index = 0;
            termFilterPtr_->println(TX::NoFrameCounterEntry);
            interpretState = idle;
        } else {
            ModemController::triggerSinglePlcRequestWithArgs(
                    PLC_CommandTypeSpecification::getMacAttribute, 3, channel0, macDeviceTable, ++index);
            interpretState = searchFCEntry;
        }
        break;
    case waitBGAutoGroupSet:
        if (hasWaitNTimes(5)) {
            interpretState = processBGAutoGroupSet;
        }
        break;
    case processBGAutoGroupSet:
        msg = assignFlowRef_.processAssignment(AssignmentFlow::plcMsg);
        if (*msg != 0) {
            termFilterPtr_->println(msg);
        }
        interpretState = idle;
        break;
    case aliveReEntryMessage:
        if (hasWaitNTimes(2)) {
          aliveJumpMapper_.transmitJumpRequestAndInform();
            interpretState = idle;
        }
        break;
    case aliveSendReEntryReply:
        if (hasWaitNTimes(1)) {
            aliveJumpMapper_.transmitJumpReply();
            interpretState = idle;
        }
        break;
    }

    msg = assignFlowRef_.processAssignment(AssignmentFlow::timerBase);
    if (*msg != 0) {
        termFilterPtr_->println(msg);
    }
}

// void ModemInterpreter::routeToAllAktivNetMembersFromNetID(const uint8_t netID) {
//    if(targetRouting.UpdateIndex == netID){
//        targetRouting.UpdateIndex++;
//    }
//    if( (targetRouting.UpdateIndex <= aktivNetMemberCount) &&
//         networkMemberList[targetRouting.UpdateIndex].isAktiv()) {
//      requestRoutingFromSourceToTargetPeer(netID,targetRouting.UpdateIndex);
//  } else {
//      targetRouting.UpdateIndex = 1;
//      targetRouting.MemberIndex++;
//      return;
//  }
// }

// void ModemInterpreter::routeAllAktivNetMembersToNetID(uint8_t& netID) {
//  if(targetRouting.UpdateIndex == netID){
//      targetRouting.UpdateIndex++;
//      return;
//  }
//  if( (targetRouting.UpdateIndex <= aktivNetMemberCount) &&
//      networkMemberList[targetRouting.UpdateIndex].isAktiv()) {
//      requestRoutingFromSourceToTargetPeer(targetRouting.UpdateIndex,netID);
//  } else {
//      targetRouting.UpdateIndex = 1;
//      netID++;
//      return;
//  }
// }

// void ModemInterpreter::requestRoutingFromSourceToTargetPeer(
//        uint8_t sourcePeerIndex, uint8_t targetPeerIndex) {
//    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
//            TX::getText(TX::cTxtRoutingReqPeer2Peer), sourcePeerIndex, targetPeerIndex);
//    termFilterPtr_->println(Global2::OutBuff);
//    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%sU%d", commandPrefix, targetPeerIndex);
//    // ModemController::triggerMessageToNetMemberWithRoutingMode(Global2::OutBuff,sourcePeerIndex,false);
//    lastTargetRoutingPeer = targetPeerIndex;
//    interpretState = waitAnnounceAnswer;
// }


bool ModemInterpreter::hasWaitNTimes(uint32_t N) {
    static uint32_t waitCOunt = 0;
    if (waitCOunt++ > N) {
        waitCOunt = 0;
        return true;
    } else {
        return false;
    }
}

// void ModemInterpreter::updateRouteToNeighbours() {
//  static enum {reduceRoutingTime, waitReducing, updateNextInaktiveRoute,
//               waitForRoutingAnswer,idle} updateStep = reduceRoutingTime;
//  static uint8_t shortAdressIndex=1;
//  static uint8_t answerTimeOut=0;
//  switch(updateStep){
//  case reduceRoutingTime:
//      G3Channel0->setADPIBAttribute(0x11, 0, 10);
//      updateStep = updateNextInaktiveRoute;
//      break;
//  case waitReducing:
//      if(answerTimeOut++ > 25){
//          updateStep = updateNextInaktiveRoute;
//          answerTimeOut = 0;
//      }
//      break;
//  case updateNextInaktiveRoute:
//      while(networkMemberList[shortAdressIndex].isAktiv()){
//          if (++shortAdressIndex >= 8) {
//              shortAdressIndex = 1;
//              updateStep = updateNextInaktiveRoute;
//              break;
//          }
//      }
// #ifdef DEBUG
//      char nextRoutingMemberText[40];
//      sprintf(nextRoutingMemberText,"Starte Routen-Suche fuer Teilnehmer %d",shortAdressIndex );
//      ModemInterpreter::modemCrierPtr->println(nextRoutingMemberText);
//      ModemInterpreter::modemCrierPtr->println(networkMemberList[shortAdressIndex].toString());
// #endif
//      G3Channel0->doRouteDiscovery(shortAdressIndex);
//      updateStep = waitForRoutingAnswer;
//      break;
//  case waitForRoutingAnswer:
//      if (isLastRoutingSuccessful()){
//          networkMemberList[shortAdressIndex].setIsAktiv(true);
//          updateStep = updateNextInaktiveRoute;
//          lastRoutingState = routeIdle;
// #ifdef DEBUG
//          ModemInterpreter::modemCrierPtr->println(ModemCrier::RoutingSucces);
// #endif
//      } else if (lastRoutingState == invalidParam || lastRoutingState == routeFailure) {
//          updateStep = updateNextInaktiveRoute;
//          shortAdressIndex++;
//          answerTimeOut = 0;
//          break;
//      }
//      if (answerTimeOut++ > 25) {
//          updateStep = updateNextInaktiveRoute;
//          shortAdressIndex++;
//          answerTimeOut = 0;
// #ifdef DEBUG
//          ModemInterpreter::modemCrierPtr->println("Route Discover Timeout");
// #endif
//
//  }
//  break;
//  case idle:break;
//  }
//
//}
//

uint32_t ModemInterpreter::getFrameCounter() {
    return frameCounter;
}

bool ModemInterpreter::isRequestForThisModul(const char *request) {
    enum {
        commandLetterOffset = 1
    };
    const char *paramPtr = request + commandLetterOffset;
    uint16_t requestedIP = (uint16_t) atoi(paramPtr);
    uint16_t currentIP = ModemController::getNetworkIPAddress();
    return currentIP == requestedIP;
}

}  // namespace AME_SRC

