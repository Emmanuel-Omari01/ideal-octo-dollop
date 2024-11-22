/*
 * PlcCommand.h
 *
 *  Created on: 16.02.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_PLCCOMMANDTYPESPECIFICATION_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_PLCCOMMANDTYPESPECIFICATION_H_

#include <cstdint>

namespace PLC_CommandTypeSpecification {

typedef enum {
    systemBlock = 0x0, g3Block = 0x1
} PLCBlock;

typedef enum {
    channel0 = 0x0, channel1 = 0x1,
} CHANNEL;

typedef enum {
    PEER = 0x0, COORDINATOR = 0x1,
} CPX_MODE;

typedef enum {
    request = 0x0, confirm = 0x1, indication = 0x2,
} ACESSTyp;

typedef enum {
    g3Controller = 0x00, umacLayer = 0x03, adpLayer = 0x04, eapLayer = 0x05,
} LAYER;

typedef enum {
    CENELEC_A = 0x00, CENELEC_B = 0x01, ARIB = 0x02, FCC = 0x03
} Frequencies;

enum ADP_layerStatusCode {
    ADP_STATUS_SUCCESS,
    ADP_STATUS_INVALID_PARAMETER = 0xE8,
    ADP_STATUS_NO_BEACON = 0XEA,
    ADP_STATUS_UNSUPPORTED_ATTRIBUTE = 0xF4,
    ADP_STATUS_INVALID_INDEX = 0xF9,
    ADP_STATUS_READ_ONLY = 0xFB,
    ADP_STATUS_INVALID_REQUEST = 0x80,
    ADP_STATUS_INVALID_IPV6_FRAME = 0x82,
    ADP_STATUS_ROUTE_ERROR = 0x83,
    STATUS_NOT_PERMITTED = 0x84,
    ADP_STATUS_TIMEOUT = 0x86,
    ADP_STATUS_ALREADY_IN_PROGRESS = 0x87,
    ADP_STATUS_INCOMPLETE_PATH = 0x88,
    ADP_STATUS_REQ_QUEUE_FULL = 0x92,
    ADP_STATUS_FAILED = 0x93,
    ADP_STATUS_CONFIG_ERROR = 0x95,
    ADP_STATUS_INSUFFICIENT_MEMSIZE = 0xA0,
    ADP_STATUS_IF_NO_RESPONSE = 0xA2,
};

enum EAP_layerStatusCode {
    R_EAP_STATUS_SUCCESS = 0x00, R_EAP_STATUS_EAP_PSK_FAILURE = 0x9A,
//  R_EAP_STATUS_xxxxxx = 0xE9,  // not documented state
};

enum confirmationPosition {
    G3_INIT,
    G3_DEINIT,
    G3_GETCONFIG,
    G3_SETCONFIG,
    G3_CLEARINFO,
    G3_GETINFO,
    G3_DUMP,
    G3_DUMP_ABORT,
    MCPS_DATA,
    MLME_RESET,
    MLME_GET,
    MLME_SET,
    MLME_SCAN,
    MLME_START,
    ADPD_DATA,
    ADPM_RESET,
    ADPM_DISCOVERY,
    ADPM_NETWORK_START,
    ADPM_NETWORK_JOIN,
    ADPM_NETWORK_LEAVE,
    ADPM_GET,
    ADPM_SET,
    ADPM_ROUTE_DISCOVERY,
    ADPM_PATH_DISCOVERY,
    ADPM_LBP,
    EAPM_RESET,
    EAPM_START,
    EAPM_GET,
    EAPM_SET,
    EAPM_NETWORK,
    EAPM_SETCLIENTINFO
};

enum eapAttributeID {
    eapGMK = 0x0,
    eapClientInfoTable = 0x04,
    onetimeClientInfo = 0x0A,
    eapEapPskRetryNum = 0x08,
};

enum adpAttributeID {
    adpSecurityLevel = 0x00,
    adpPrefixTable = 0x01,
    adpBroadcastLogTableEntryTTL = 0x02,
    adpMetricType = 0x03,
    adpLowLQIValue = 0x04,
    adpHighLQIValue = 0x05,
    adpRREPWait = 0x06,
    adpContextInformationTable = 0x07,
    adpCoordShortAddress = 0x08,
    adpRLCEnabled = 0x09,
    adpAddRevLinkCost = 0x0A,
    adpBroadcastLogTable = 0x0B,
    adpRoutingTable = 0x0C,
    adpUnicastRREQGenEnable = 0x0D,
    adpGroupTable = 0x0E,
    adpMaxHops = 0x0F,
    deviceType = 0x10,
    adpNetTraversalTime = 0x11,
    adpRoutingTableEntryTTL = 0x12,
    adpKr = 0x13,
    adpKm = 0x14,
    adpKc = 0x15,
    adpKq = 0x16,
    adpKh = 0x17,
    adpRREQRetries = 0x18,
    adpRREQWait = 0x19,
    adpWeakLQIValue = 0x1A,
    adpKrt = 0x1B,
    adpSoftVersion = 0x1C,
//  adpSnifferMode = 0x1D,          // not supported -> Status 0xF4
    adpBlacklistTable = 0x1E,
    adpBlacklistTableEntryTTL = 0x1F,
    adpMaxJoinWaitTime = 0x20,
    adpPathDiscoveryTime = 0x21,
    adpActiveKeyIndex = 0x22,
    adpDestinationAddressSet = 0x23,
    defaultCoordRouteEnabled = 0x24,
    adpDisableDefaultRouting = 0xF0
};

const uint8_t adpFetchAttributes[] = {  // const enum adpAttributeID ...
        adpSecurityLevel, adpPrefixTable, adpBroadcastLogTableEntryTTL,
                adpMetricType, adpLowLQIValue, adpHighLQIValue, adpRREPWait,
                adpContextInformationTable, adpCoordShortAddress, adpRLCEnabled,
                adpAddRevLinkCost, adpBroadcastLogTable, adpRoutingTable,
                adpUnicastRREQGenEnable, adpGroupTable, adpMaxHops, deviceType,
                adpNetTraversalTime, adpRoutingTableEntryTTL, adpKr, adpKm,
                adpKc, adpKq, adpKh, adpRREQRetries, adpRREQWait,
                adpWeakLQIValue, adpKrt, adpSoftVersion, adpBlacklistTable,
                adpBlacklistTableEntryTTL, adpMaxJoinWaitTime,
                adpPathDiscoveryTime, adpActiveKeyIndex,
                adpDestinationAddressSet, defaultCoordRouteEnabled,
                adpDisableDefaultRouting };

enum umacAttributeID {
    macAckWaitDuration = 0x0040,
    macMaxBE = 0x0047,
    macBSN = 0x0049,
    macDSN = 0x004C,
    macMaxCSMABackoffs = 0x004E,
    macMinBE = 0x004F,
    macPanId = 0x0050,
    macPromiscuousMode = 0x0051,
    macShortAddress = 0x0053,
    macMaxFrameRetries = 0x0059,
    macTimeStampSupported = 0x005C,
    macSecurityEnabled = 0x005D,
    macKeyTable = 0x0071,
    macDeviceTable = 0x0073,
    macFrameCounter = 0x0077,
    macHighPriorityWindowSize = 0x0100,
    macTxDataPacketCount = 0x0101,
    macRxDataPacketCount = 0x0102,
    macTxCmdPacketCount = 0x0103,
    macRxCrndPacketCount = 0x0104,
    macCSMAFailCount = 0x0105,
    macCSMAnoACKCount = 0x0106,
    macRxDataBroadcastCount = 0x0107,
    macTxDataBroadcastCount = 0x0108,
    macBadCRCCount = 0x0109,
    macNeighbourTable = 0x010A,
    macCSMAFairnessLimit = 0x010C,
    macTMRTTL = 0x010D,
    macPOSTableEntryTTL = 0x010E,
    macRCCoord = 0x010F,
    macToneMask = 0x0110,
    macBeaconRandomizationWindowLength = 0x0111,
    macA = 0x0112,
    macK = 0x0113,
    macMinCWattempls = 0x0114,
    macCENELECLegacyMode = 0x0115,
    macFCCLegacyMode = 0x0116,
    macBroadcastMaxCWEnable = 0x011E,
    macTransmitAtten = 0x011F,
    macPOSTable = 0x0120,
};
const uint16_t umacFetchAttributes[] = {  // const enum umacAttributeID ...
        macAckWaitDuration, macMaxBE, macBSN, macDSN, macMaxCSMABackoffs,
                macMinBE, macPanId, macPromiscuousMode, macShortAddress,
                macMaxFrameRetries, macTimeStampSupported, macSecurityEnabled,
                macDeviceTable, macFrameCounter, macHighPriorityWindowSize,
                macTxDataPacketCount, macRxDataPacketCount, macTxCmdPacketCount,
                macRxCrndPacketCount, macCSMAFailCount, macCSMAnoACKCount,
                macRxDataBroadcastCount, macTxDataBroadcastCount,
                macBadCRCCount, macNeighbourTable, macCSMAFairnessLimit,
                macTMRTTL, macPOSTableEntryTTL, macRCCoord, macToneMask,
                macBeaconRandomizationWindowLength, macA, macK,
                macMinCWattempls, macCENELECLegacyMode, macFCCLegacyMode,
                macBroadcastMaxCWEnable, macTransmitAtten, macPOSTable };

typedef enum {
    INIT,
    DEINIT,
    GETCONFIG,
    SETCONFIG,
    CLEARINFO = 0x14,
    GETINFO = 0x15,
    DUMP = 0x18,
    DUMP_ABORT = 0x19,
    EVENT = 0x20
} CTRL_CMD_ID;

typedef enum {
    DATA,
    RESET,
    DISCOVERY,
    NETWORK_START,
    NETWORK_JOIN,
    NETWORK_LEAVE,
    GET,
    SET,
    ROUTE_DISCOVERY,
    PATH_DISCOVERY,
    LBP,
    NETWORK_STATUS,
    BUFFER,
    KEY_STATE,
    ROUTE_ERROR,
    ADP_KEY
} ADP_CMD_ID;

typedef enum {
    EAP_RESET,
    EAP_START,
    EAP_GET,
    EAP_SET,
    EAP_NETWORK,
    EAP_SETCLIENTINFO,
    EAP_JOIN,
    EAP_LEAVE,
    EAP_NEWDEVICE,
    EAP_KEY
} EAP_CMD_ID;

typedef enum {
    UMAC_DATA,
    UMAC_RESET,
    UMAC_GET,
    UMAC_SET,
    SCAN,
    START,
    BEACON_NOTIFY,
    COMM_STATUS,
    FRAMECOUNT,
    TMR_RECEIVE,
    TMR_TRANSMIT
} UMAC_CMD_ID;

typedef enum {
    SYSTEM_PING,
    SYSTEM_VERSION_GET,
    SYSTEM_RESET,
    SYSTEM_BOOTUP,
    SYSTEM_EVENT,
    SYSTEM_REBOOT,
    SYSTEM_CLEARINFO = 0x10,
    SYSTEM_GETINFO,
    SYSTEM_SROM_READ = 0x20,
    SYSTEM_SROM_WRITE,
    SYSTEM_SROM_ERASE,
} SYSTEM_CMD_ID;

typedef enum {
    channelInit,
    setConfig,
    adpReset,
    adpSet,
    eapReset,
    eapSet,
    networkStart,
    eapmStart,
    setClientInfo,
    routeDiscovery,
    pathDiscovery,
    sendData,
    searchNetwork,
    joinNetwork,
    forceJoinNetwork,
    getEapAttribute,
    getAdpAttribute,
    getMacAttribute,
    setMacAttribute,
    setAdpAttribute,
    kickPeer,
    setEapAttribute,
    noRequest,
    leaveNetwork,
    getSystemVersion,
} plcRequestNames;

typedef enum {
    DefaultCoordChannel0,
    DeviceTypeChannel0,
    OneTimeChannel0,
    pskRetryNumChannel0,
    eapGMKChannel0,
    ClearDeviceFrameCounter,
} plcUserSetReqNames;
}  // namespace PLC_CommandTypeSpecification

typedef struct {
    PLC_CommandTypeSpecification::plcRequestNames requestName;
    unsigned char argAmount;
    uint32_t *commandCreationArgs;
    const char *message;
    unsigned char confirmPosition;
} plcExecuteData;

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_PLCCOMMANDTYPESPECIFICATION_H_
