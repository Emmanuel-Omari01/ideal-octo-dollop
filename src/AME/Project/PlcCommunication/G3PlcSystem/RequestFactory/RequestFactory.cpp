/*
 * RequestFactory.cpp
 *
 *  Created on: 20.07.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "RequestFactory.h"

#include <cstring>
#include <cstdarg>

#include "../../ModemController.h"
#include "../../../SystemBase/VersionKey.h"


namespace AME_SRC {
// RequestFactory::plcRequestNames RequestFactory::requestName =  noRequest;
PlcCommand RequestFactory::tmpCommandContainer = PlcCommand();
CHANNEL RequestFactory::channelID = channel0;
RequestFactory::g3PramStruct RequestFactory::g3Parameter = {
// coordinator parameter
        { eapLayer,  // g3mode
                0x00, ModemController::networkMemberMax,  // The number of neighbour table
                0x00, ModemController::networkMemberMax,  // The number of device table
                0x00, 0x40,  // panDescriptorNum
                0x00, 0x0A,  // routeTableSize
                0x00, 0x04,  // adpdBuffNum
                0x00,  // routeType
                0x00, 0x80,  // eapLbpBuffNum max 128 =>
                             // The number of elements in the LBP message buffer.
                             // Maximum number of EAP-PSK (Join) process which can be run in parallel.
                0x00, ModemController::networkMemberMax,  // eapCinfoTableNum
        },
        // peerParameter
        { adpLayer,  // g3mode
                0x00, ModemController::networkMemberMax,  // The number of neighbour table
                0x00, ModemController::networkMemberMax,  // The number of device table
                0x00, 0x40,  // panDescriptorNum
                0x00, 0x0A,  // routeTableSize
                0x00, 0x04,  // adpdBuffNum
                0x00,  // routeType
        },
        // config
        { adpLayer,  // g3mode
                PLC_CommandTypeSpecification::FCC,  // CENELEC A
                0x00, 0x00, 0x00, 0x00,  // Reserve area
                0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x02,  // extendedAddress // 0xFF0102FFFE000101
                AME_PSK,                                         // PSK
                0x00,  // extIDFlag
        },
        // GMK-0
        { 0xAF, 0x4D, 0x6D, 0xCC, 0xF1, 0x4D, 0xE7, 0xC1, 0xC4, 0x23, 0x5E,
                0x6F, 0xEF, 0x6C, 0x15, 0x1F },  // 0x1F auf 0x00
        // defaultclientInfo
        { 0x01,  // validFlag
                0xFF, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x01, 0x02,  // extendedAddress
                0x00,        // accessControl
                0x00, 0x01,  // networkAddress
                AME_PSK,     // psk
                0x00,        // extIdFlag
        } };


RequestFactory::RequestFactory() = default;

RequestFactory::~RequestFactory() = default;

PlcCommand& RequestFactory::createRequest(plcRequestNames requestName, int count, ...) {
    va_list args;
    va_start(args, count);
    createRequest(requestName, args);
    va_end(args);
    return tmpCommandContainer;
}

PlcCommand& RequestFactory::createRequest(plcRequestNames requestName,
                                          va_list args) {
    uint8_t cmdID, position;
    LAYER targetLayer;
    channelID = (CHANNEL) va_arg(args, int);
    switch (requestName) {
        case channelInit:
            fillTmpCmdWithInitReq(args);
            break;
        case setConfig:
            // TODO(AME): make FCC & CENELEC A switchable
            fillTmpCmdWithSetConfigReq(args);
            break;
        case adpReset:
            fillTmpCmdWithADPResetReq();
            break;
        case setAdpAttribute:
            fillTmpCmdWithADPSetReq(args);
            break;
        case eapReset:
            fillTmpCmdWithEAPResetReq();
            break;
        case setEapAttribute:
            fillTmpCmdWithEAPSetReq(args);
            break;
        case kickPeer:
            fillTmpCmdWithEAPNetworkReq(args);
            break;
        case networkStart:
            fillTmpCmdWithADPNetworkStart(args);
            break;
        case eapmStart:
            fillTmpCmdWithEAPStartReq();
            break;
        case setClientInfo:
            fillTmpCmdWithEAPSetClientInfoReq(args);
            break;
        case sendData:
            fillTmpCmdWithADPSend(args);
            break;
        case routeDiscovery:
            fillTmpCmdWithADPRouteDiscoveryReq(args);
            break;
        case pathDiscovery:
            fillTmpCmdWithADPPathDiscoveryReq(args);
            break;
        case getEapAttribute:
            fillTmpCmdWithEAPGetReq(args);
            break;
        case getAdpAttribute:
            fillTmpCmdWithADPGetReq(args);
            break;
        case getMacAttribute:
            fillTmpCmdWithMACGetReq(args);
            break;
        case setMacAttribute:
            fillTmpCmdWithMACSetReq(args);
            break;
        case searchNetwork:
            fillTmpCmdWithADPSearchNetwork(args);
            break;
        case joinNetwork:
            fillTmpCmdWithADPJoin(args);
            break;
        case leaveNetwork:
            fillTmpCmdWithADPLeave();
            break;
        case forceJoinNetwork:
            fillTmpCmdWithADPForceJoin(args);
            break;
        case getSystemVersion:
            fillTmpCmdWithGetSystemVersion();
        break;
        default: {
        }
    }
    tmpCommandContainer.calculateCRC();
    tmpCommandContainer.incCmdNr();
    cmdID = tmpCommandContainer.getCommandID();
    targetLayer = tmpCommandContainer.getAccesLayer();
    position = getConfirmPosition(targetLayer, cmdID);
    tmpCommandContainer.setConfirmIndex(position);
    return tmpCommandContainer;
}

PlcCommand& RequestFactory::createUserSetRequest(plcUserSetReqNames userSetCmdName, int count, ...) {
    uint8_t argAmount = 5;
    uint8_t tableIndex = 0;
    uint8_t dataLength = 0;
    uint8_t dataStorage[10] = { 0 };
    va_list args;
    va_start(args, count);
    switch (userSetCmdName) {
        case DefaultCoordChannel0:
            dataLength = 1;
            dataStorage[0] = (uint8_t) va_arg(args, int);
            va_end(args);
            createRequest(setAdpAttribute, argAmount,
                    channel0, defaultCoordRouteEnabled, tableIndex, dataLength, (uint32_t) &dataStorage);
            break;
        case DeviceTypeChannel0:
            dataLength = 1;
            dataStorage[0] = (uint8_t) va_arg(args, int);
            createRequest(setAdpAttribute, argAmount,
                    channel0, deviceType, tableIndex, dataLength, (uint32_t) &dataStorage);
            break;
        case OneTimeChannel0:
            dataLength = 1;
            dataStorage[0] = (uint8_t) va_arg(args, int);
            createRequest(setEapAttribute, argAmount,
                    channel0, onetimeClientInfo, tableIndex, dataLength, (uint32_t) &dataStorage);
            break;
        case pskRetryNumChannel0:
            dataLength = 2;
            uint8Operations::insertUint16intoUint8Array(dataStorage, (uint16_t) va_arg(args, int));
            createRequest(setEapAttribute, argAmount,
                    channel0, eapEapPskRetryNum, tableIndex, dataLength, (uint32_t) &dataStorage);
            break;
        case eapGMKChannel0:
            createRequest(setEapAttribute, argAmount, channel0, eapGMK, tableIndex,
                    sizeof(g3Parameter.GMK0), (uint32_t) &g3Parameter.GMK0[0]);
            break;
        case ClearDeviceFrameCounter:
            dataLength = 10;
            tableIndex = (uint8_t) va_arg(args, int);
            uint8Operations::insertUint16intoUint8Array(dataStorage, (uint16_t) va_arg(args, int));
            createRequest(setMacAttribute, argAmount, channel0, macDeviceTable,
                tableIndex, dataLength, (uint32_t) &dataStorage);
            break;
        default: {
        }
    }
    return tmpCommandContainer;
}

#define CTRL_CMD_AREA
#ifdef CTRL_CMD_AREA

/***
 * Creates an initialization command for a channel
 * @param channel Channel to which the command is related
 * @param mode Mode to be initialized
 * @return Command to initialize a channel
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithInitReq(va_list args) {
    CPX_MODE initDeviceMode = (CPX_MODE) va_arg(args, int);
    byteArray paramSettings;
    getRecommendedParmForMode(initDeviceMode, &paramSettings);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, g3Controller,
                                     PLC_CommandTypeSpecification::INIT, paramSettings.ptr,
                                     (uint8_t) paramSettings.size);  // 0x00 = CMD = G3-Init.request
    tmpCommandContainer.setConfirmIndex(G3_INIT);
}

void RequestFactory::getRecommendedParmForMode(CPX_MODE mode,
        RequestFactory::byteArray *recommendedParamSettings) {
    if (mode == COORDINATOR) {
        recommendedParamSettings->ptr = reinterpret_cast<uint8_t*>(g3Parameter.coordinatorParameter);
        recommendedParamSettings->size = sizeof(g3Parameter.coordinatorParameter);
    } else {
        recommendedParamSettings->ptr = reinterpret_cast<uint8_t*>(g3Parameter.peerParameter);
        recommendedParamSettings->size = sizeof(g3Parameter.peerParameter);
    }
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithSetConfigReq(va_list args) {
    enum { extendedAdressPosition = 6 };
    uint8_t *extendedAdress = va_arg(args, uint8_t*);
    memcpy(reinterpret_cast<void *>(&g3Parameter.config[extendedAdressPosition]), extendedAdress, 8);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, g3Controller,
                                     SETCONFIG, reinterpret_cast<uint8_t*>(g3Parameter.config),
                                     sizeof(g3Parameter.config));
    tmpCommandContainer.setConfirmIndex(G3_SETCONFIG);
}

#endif

#define ADP_CMD_AREA
#ifdef ADP_CMD_AREA

/***
 * Creates a PLC command that forwards an NSDU packet to the PLC modem
 * @param channel Channel to which the command is related
 * @param nsduPtr pointer to the "Network Service Data Unit" which contains the IP-V6 frame, UDP and data.
 * @param nsduLength length of "Network Service Data Unit"
 * @param routeDiscovery Flag to activate a route search before sending an NSDU frame
 * @return Command to send a NSDU frame
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPSend(va_list args) {
    uint8_t *nsduPtr = va_arg(args, uint8_t*);
    uint8_t nsduLength = (uint8_t) va_arg(args, int);
    bool routingMode = va_arg(args, int);
    bool qualityOfService = va_arg(args, int);
    // DS- uint8_t requestTailLength = 5;
    nsduPtr[0] = 0;
    nsduPtr[1] = nsduLength;
    nsduPtr[nsduLength + 2] = 0x00;
    nsduPtr[nsduLength + 3] = routingMode;
    nsduPtr[nsduLength + 4] = qualityOfService;
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer,
                                     DATA, nsduPtr, (uint8_t) (nsduLength + 5));
    tmpCommandContainer.setConfirmIndex(ADPD_DATA);
}

/***
 * Creates a PLC command with which the ADP layer can be restarted
 * @param channel Channel on which the ADP layer should be restarted
 * @return Command to restart
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPResetReq() {
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer, RESET, 0, 0);
    tmpCommandContainer.setConfirmIndex(ADPM_RESET);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPSetReq(va_list args) {
    byteArray adpSetArgs;
    prepareSetDataArgsForLayer(args, adpLayer, &adpSetArgs);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer, SET,
                                     adpSetArgs.ptr, (uint8_t) adpSetArgs.size);
    tmpCommandContainer.setConfirmIndex(ADPM_SET);
}
void RequestFactory::prepareGetDataArgsForLayer(va_list args, LAYER layer,
        RequestFactory::byteArray *result) {
    enum {
        tableOffset = 2
    };
    result->ptr = NULL;
    result->size = 0;
    uint8_t attributeIDoffset = 0;
    if (layer == umacLayer) {
        uint16_t attributeID = (uint16_t) va_arg(args, int);
        uint8Operations::insertUint16intoUint8Array(
                reinterpret_cast<uint8_t*>(Global2::OutBuff), attributeID);
        attributeIDoffset = 2;
    } else if (layer == adpLayer || layer == eapLayer) {
        uint8_t attributeID = (uint8_t) va_arg(args, int);
        Global2::OutBuff[0] = attributeID;
        attributeIDoffset = 1;
    }
    uint16_t tableIndex = (uint16_t) va_arg(args, int);
    uint8Operations::insertUint16intoUint8Array(
            reinterpret_cast<uint8_t*>(Global2::OutBuff + attributeIDoffset), tableIndex);
    result->ptr = reinterpret_cast<uint8_t*>(Global2::OutBuff);
    result->size = attributeIDoffset + tableOffset;
}

void RequestFactory::prepareSetDataArgsForLayer(va_list args, LAYER layer,
        RequestFactory::byteArray *result) {
    enum {
        tableOffset = 2
    };
    result->ptr = NULL;
    result->size = 0;
    uint8_t attributeIDoffset = 0;
    if (layer == umacLayer) {
        uint16_t attributeID = (uint16_t) va_arg(args, int);
        uint8Operations::insertUint16intoUint8Array(reinterpret_cast<uint8_t*>(Global2::OutBuff), attributeID);
        attributeIDoffset = 2;
    } else if (layer == g3Controller || layer == adpLayer || layer == eapLayer) {
        uint8_t attributeID = (uint8_t) va_arg(args, int);
        Global2::OutBuff[0] = attributeID;
        attributeIDoffset = 1;
    }

    uint16_t tableIndex = (uint16_t) va_arg(args, int);
    uint8_t dataLength = (uint8_t) va_arg(args, int);
    uint8_t *data = va_arg(args, uint8_t*);
    uint8Operations::insertUint16intoUint8Array(
            reinterpret_cast<uint8_t*>(Global2::OutBuff + attributeIDoffset), tableIndex);
    memcpy(Global2::OutBuff + attributeIDoffset + tableOffset, data,
            dataLength);
    result->size = attributeIDoffset + tableOffset + dataLength;
    result->ptr = reinterpret_cast<uint8_t*>(Global2::OutBuff);
}

/***
 * Creates a PLC command to read any ADP attribute
 * @param channel Channel to which the command is related
 * @param identifier  ID of attribute which is to be read
 * @param tableIndex  Index of the table row, which is to be read (will be ignored with attribute without table type)
 * @return command to read an ADP attribute
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPGetReq(va_list args) {
    byteArray getParam;
    prepareGetDataArgsForLayer(args, eapLayer, &getParam);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer, GET,
                                     getParam.ptr, (uint8_t) getParam.size);
    tmpCommandContainer.setConfirmIndex(ADPM_GET);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPNetworkStart(va_list args) {
    uint16_t panId = (uint16_t) va_arg(args, int);
    uint8_t panIdContainer[2];
    uint8Operations::insertUint16intoUint8Array(panIdContainer, panId);
    PLC_CommandTypeSpecification::ADP_CMD_ID comandName = NETWORK_START;
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer, comandName, panIdContainer, 2);
    tmpCommandContainer.setConfirmIndex(ADPM_NETWORK_START);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPRouteDiscoveryReq(va_list args) {
    byteArray discoveryParam;
    combineUint16ArgWithUint8Arg(args, &discoveryParam);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer,
                                     ROUTE_DISCOVERY, discoveryParam.ptr, (uint8_t) discoveryParam.size);
    tmpCommandContainer.setConfirmIndex(ADPM_ROUTE_DISCOVERY);
}

void RequestFactory::combineUint16ArgWithUint8Arg(va_list args, RequestFactory::byteArray *result) {
    result->ptr = reinterpret_cast<uint8_t*>(Global2::OutBuff);
    result->size = 0;

    uint16_t firstArg = (uint16_t) va_arg(args, int);
    uint8_t secondArg = (uint8_t) va_arg(args, int);
    uint8Operations::insertUint16intoUint8Array(result->ptr, firstArg);
    result->ptr[2] = secondArg;
    result->size = 3;
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPPathDiscoveryReq(va_list args) {
    byteArray pathDiscoveryParam;
    combineUint16ArgWithUint8Arg(args, &pathDiscoveryParam);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer,
                                     PATH_DISCOVERY, pathDiscoveryParam.ptr,
                                     (uint8_t) pathDiscoveryParam.size);
    tmpCommandContainer.setConfirmIndex(ADPM_PATH_DISCOVERY);
}

/***
 * Creates a PLC command that searches for accessible PAN IDs
 * @param channel Channel to which the command is related
 * @param seconds  Number of seconds the scan should last
 * @return Command to create a PAN search query
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPSearchNetwork(va_list args) {
    uint8_t scanTime = (uint8_t) va_arg(args, int);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer, DISCOVERY, &scanTime, 1);
    tmpCommandContainer.setConfirmIndex(ADPM_DISCOVERY);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPJoin(va_list args) {
    uint16_t panId = (uint16_t) va_arg(args, int);
    uint16_t lbAddress = (uint16_t) va_arg(args, int);
    uint8_t joinParam[4] = { 0x0, 0x0, 0x0, 0x0 };
    joinParam[0] = (uint8_t) (panId >> 8);
    joinParam[1] = (uint8_t) panId & 0x00FF;
    joinParam[2] = (uint8_t) (lbAddress >> 8);  // ToDo(DS): check. shift command inserted here.
    joinParam[3] = (uint8_t) lbAddress & 0x00FF;
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer,
                                     NETWORK_JOIN, joinParam, 4);
    tmpCommandContainer.setConfirmIndex(ADPM_NETWORK_JOIN);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPLeave(void) {
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer, NETWORK_LEAVE, 0, 0);
    tmpCommandContainer.setConfirmIndex(ADPM_NETWORK_JOIN);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithADPForceJoin(va_list args) {
    uint16_t targetNetId = (uint16_t) va_arg(args, int);
    uint16_t ownNetworkAddress = (uint16_t) va_arg(args, int);
    uint8_t joinParam[39] = { 0 };
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, adpLayer, 0xF0, joinParam, 39);
    tmpCommandContainer.setReserveByte(0x01);
    uint8Operations::insertUint16intoUint8Array(joinParam, targetNetId);
    uint8Operations::insertUint16intoUint8Array(joinParam + 2, ownNetworkAddress);
    joinParam[4] = 0;
    memcpy(joinParam + 5, g3Parameter.GMK0, 16);
    uint8Operations::insertUint16intoUint8Array(joinParam + 37, 0x18);
    tmpCommandContainer.copyNBytesFromArrayToParameter(sizeof(joinParam), joinParam);
    tmpCommandContainer.setConfirmIndex(ADPM_NETWORK_JOIN);
}

#endif

#define EAP_CMD_AREA
#ifdef EAP_CMD_AREA

/***
 * Creates a PLC command that triggers the authentication process for new participants
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithEAPStartReq() {
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, eapLayer, EAP_START, 0, 0);
    tmpCommandContainer.setConfirmIndex(EAPM_START);
}

/***
 * Creates a PLC command with which the ADP layer can be restarted
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithEAPResetReq() {
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, eapLayer, EAP_RESET, 0, 0);
    tmpCommandContainer.setConfirmIndex(EAPM_RESET);
}

/***
 * Creates a PLC command that reads an attribute of the EAP layer
 * @param channel Channel to which the command is related
 * @param identifier  index of the EAP attribute
 * @return command to read an EAP attribute
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithEAPGetReq(va_list args) {
    byteArray getParam;
    prepareGetDataArgsForLayer(args, eapLayer, &getParam);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, eapLayer,
                                     EAP_GET, getParam.ptr, (uint8_t) getParam.size);
    tmpCommandContainer.setConfirmIndex(EAPM_GET);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithEAPSetReq(va_list args) {
    byteArray eapSetArgs;
    prepareSetDataArgsForLayer(args, eapLayer, &eapSetArgs);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, eapLayer,
                                     EAP_SET, eapSetArgs.ptr, (uint8_t) eapSetArgs.size);
    tmpCommandContainer.setConfirmIndex(EAPM_SET);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithEAPNetworkReq(va_list args) {
    // TODO(AME): repair MAC address
    uint8_t *extendedAddress = va_arg(args, uint8_t*);
    uint8_t networkParam[11] = { 0x0/*GMKID*/, 0x1/*Kick*/, 0x0/*ReqHandle*/,
            0xFF, 0x0, 0x02, 0x0, 0x0, 0x0, 0x0, 0x0 /*Extended address*/};
    memcpy(networkParam + 6, extendedAddress + 3, 5);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, eapLayer,
                                     EAP_NETWORK, networkParam, sizeof(networkParam));
    tmpCommandContainer.setConfirmIndex(EAPM_SETCLIENTINFO);
}

/***
 * Creates a PLC command that saves the subscriber information in the PLC modem
 * @param channel Channel to which the command is related
 * @param networkAddress Short address of the member within a PAN
 * @param extendedAddress MAC address of the member
 * @return command to set the subscriber information in the PLC modem
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithEAPSetClientInfoReq(va_list args) {
    bool isValChannelID = va_arg(args, int);
    uint16_t networkAddress = (uint16_t) va_arg(args, int);
    uint8_t *extendedAddress = va_arg(args, uint8_t*);
    g3Parameter.defaultclientInfo[0] = isValChannelID;
    memcpy(g3Parameter.defaultclientInfo + 1, extendedAddress, 8);
    uint8Operations::insertUint16intoUint8Array(
        g3Parameter.defaultclientInfo + 10, networkAddress);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, eapLayer,
                                     EAP_SETCLIENTINFO, g3Parameter.defaultclientInfo,
                                     sizeof(g3Parameter.defaultclientInfo));
    tmpCommandContainer.setConfirmIndex(EAPM_SETCLIENTINFO);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithMACGetReq(va_list args) {
    byteArray getParam;
    prepareGetDataArgsForLayer(args, umacLayer, &getParam);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, umacLayer,
                                     0x02, getParam.ptr, (uint8_t) getParam.size);
    tmpCommandContainer.setConfirmIndex(MLME_GET);
}

#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithMACSetReq(va_list args) {
    byteArray setArgs;
    prepareSetDataArgsForLayer(args, umacLayer, &setArgs);
    tmpCommandContainer = PlcCommand(g3Block, channelID, request, umacLayer,
                                     UMAC_SET, setArgs.ptr, (uint8_t) setArgs.size);
    tmpCommandContainer.setConfirmIndex(MLME_SET);
}
#endif

#define SYSTEM_BLOCK_AREA
#ifdef SYSTEM_BLOCK_AREA
#pragma GCC diagnostic ignored "-Wstack-usage="
void RequestFactory::fillTmpCmdWithGetSystemVersion() {
    tmpCommandContainer = PlcCommand(systemBlock, channelID, request,
                                     g3Controller, SYSTEM_VERSION_GET, NULL, 0);
    tmpCommandContainer.setConfirmIndex(G3_GETINFO);
}
#endif

// Determine the bit position in the acknowledgment mask of a PlcCommand
uint8_t RequestFactory::getConfirmPosition(LAYER layer, uint8_t cmdID) {
    enum {
        ControllerOffset = 8, UmacOffset = 6, AdpOffset = 11
    };
    uint8_t position = 0;
    if (layer == g3Controller) {
        switch (cmdID) {
            case INIT:
                position = 0; break;
            case SETCONFIG:
                position = 3;
                break;
            case CLEARINFO:
                position = 4;
                break;
            case DUMP:
                position = 5;
                break;
            case DUMP_ABORT:
                position = 6;
                break;
            default:
                position = cmdID;
                break;
        }
    } else if (layer == umacLayer) {
        // using namespace PLC_CommandTypeSpecification;
        position = (uint8_t) (cmdID + ControllerOffset);
    } else if (layer == adpLayer) {
        position = (uint8_t) (cmdID + ControllerOffset + UmacOffset);
    } else if (layer == eapLayer) {
        position = (uint8_t) (cmdID + ControllerOffset + UmacOffset + AdpOffset);
    }
    return position;
}

}  // namespace AME_SRC
