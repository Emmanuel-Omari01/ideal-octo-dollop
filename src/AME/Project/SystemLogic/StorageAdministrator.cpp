/*
 * StorageAdministrator.cpp
 *
 *  Created on: 28.02.2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "StorageAdministrator.h"

#include <cstdio>
#include <cstring>

#include "I18N.h"
#include "../PlcCommunication/PlcTerminalCommandConnector.h"
#include "../StorageManagement/SimpleFlashEditor.h"

namespace AME_SRC {

static CircularBufferManager circularBufferManager;
IKeyContainerStorage* StorageAdministrator::storageInterfacePtr =&circularBufferManager;
char StorageAdministrator::statusText[50];
bool StorageAdministrator::saveInfoAgainRequest = false;

StorageAdministrator::StorageAdministrator() = default;

bool StorageAdministrator::isPlcGetAttributeIdStorable(
    const PlcCommand &confirmation, LAYER layer, uint16_t attributeId,
    uint16_t *attributeIdx, const uint8_t **pData, uint8_t *data_len,
    enum StorageAdministrator::storageKeys *skey) {
    // using namespace PLC_CommandTypeSpecification;
    const uint8_t *p = confirmation.getParameter();  // &Payload
    uint8_t *pValidTime;
    bool lResult = true;
    switch (layer) {
        case umacLayer:
            *pData = p + 5;
            p += 3;
            switch (attributeId) {
                case PLC_CommandTypeSpecification::macFrameCounter:
                    //                0102030405060708091011121314151617181920212223
                    // 00 00 77 00 00 0000000100000000000000000000000000000000000000BC
                    // 00           Status
                    // 00 77        pibAttributeId
                    // 00 00        pibAttributeIndex
                    // 00 00 00 01  pibAttributeValue: macFrameCounter
                    *data_len = 4;
                    *skey = frameCounter;
                    break;
                case PLC_CommandTypeSpecification::macNeighbourTable:
                    //                 0102030405060708091011121314151617181920212223
                    //             000102030405060708091011121314151617181920212223
                    //       00010A0000060000FFFFFF3000000009380000000000003D000000009B
                    // 00 01 0A 00 00  060000FFFFFF0000000000340000000000003E0000000059
                    // 00       Status
                    // 01 0A    pibAttributeId
                    // 00 00    pibAttributeIndex
                    // 06 00 00 ... pibAttributeValue:macNeighbourTable
                    *data_len = 19;
                    *skey = macNeighbourTable;

                    // Overwrite the value of tmrValidTime with the default value of 10 (minutes)
                    // Otherwise a new entry would be created in the data flash after each reading process
                    pValidTime = (uint8_t *) (p + 9);
                    if (*((uint32_t*) pValidTime)) {  // overwrite entry if it is not 0
                            uint8Operations::insertUint32intoUint8Array(pValidTime, 10);
                    }
                    break;
                default:
                    return false;
        }
        break;
        case adpLayer:
            switch (attributeId) {
                case adpRoutingTable: {
                    //             01020304050607080910111213141516171819202122232425262728293031323334353637
                    // 00 0C 00 00 000000007FFF08080141000000000000000000000000000000000000000000000000000000E5
                    // 00                   Status
                    // 0C                   aibAttributeId
                    // 00 00                aibAttributeIndex
                    // 000000007FFF08080141  adpRouteTable
                    *data_len = 10;
                    *skey = adpRouteTable;
                    *pData = p + 4;

                    // Overwrite the value of validTime with the default value of 360 (minutes)
                    // Otherwise a new entry would be created in the data flash after each reading process
                    pValidTime = (uint8_t*) (p + 4 + 8);
                    if (*((uint16_t*) pValidTime)) {// Override the value only if not zero (0)
                        uint8Operations::insertUint16intoUint8Array(pValidTime, 360);
                    } else {
                        lResult = false;    // => don't save
                    }
                    p += 2;
                }
                break;
                default:
                    return false;
        }
        break;
//      case eapLayer:
//          return false;
//      case g3Controller:
//          return false;
    default:
        return false;
    }
    *attributeIdx = uint8Operations::sum2BytesFromLSB(p);
    return lResult;
}

void StorageAdministrator::storePlcGetConfirmationFromLayer(const PlcCommand &confirmation, LAYER layer) {
    uint16_t attributeID = layer == umacLayer ?
                    uint8Operations::sum2BytesFromLSB(
                            confirmation.getParameter() + 1) : confirmation.getParameter()[1];
    uint16_t attributeIdx;
    const uint8_t *pData;
    uint8_t data_len;
    enum storageKeys store_key;
    if (isPlcGetAttributeIdStorable(confirmation, layer, attributeID,
            &attributeIdx, &pData, &data_len, &store_key)) {
        IKeyContainerStorage::container attributeContainer = {pData,
                                                              data_len };
        storeIfNewContainer(store_key, 0, attributeIdx, attributeID,
                attributeContainer);
    } else {
        snprintf(statusText, sizeof(statusText), TX::getText(TX::GetAttributeIsNotSaved), attributeID);
    }
}


// Save container to data flash if it is updated, does not exist or is invalid
// Return: true: Container has been saved
bool StorageAdministrator::storeIfNewContainer(enum storageKeys store_key,
                                               uint8_t store_flags, uint16_t attributeIdx, uint16_t attributeID,
                                               IKeyContainerStorage::container attributeContainer) {
    IKeyContainerStorage::skey temp_key, load_key;
    temp_key.extended_key.key = store_key;
    temp_key.extended_key.index = attributeIdx;
    temp_key.extended_key.flag.all = store_flags;
    load_key = temp_key;
    IKeyContainerStorage::container loadedCon;
    storageInterfacePtr->loadContainerWithKey(&load_key, &loadedCon);
    if (!storageInterfacePtr->compare2Container(attributeContainer, loadedCon)
            || !storageInterfacePtr->compare2Flags(temp_key, load_key)) {
        storageInterfacePtr->storeKeyContainer(temp_key, attributeContainer);
        if (storageInterfacePtr->containerDeleted()) {
            SimpleFlashEditor::updateSysParamFlash();
            storageInterfacePtr->releaseContainerDeleted();
            saveInfoAgainRequest = true;
        }
        snprintf(statusText, sizeof(statusText),
                 TX::getText(TX::AttributeSaved), attributeID, TX::getText(TX::txtWas), attributeIdx);
        return true;
    } else {
        snprintf(statusText, sizeof(statusText),
                 TX::getText(TX::AttributeSaved), attributeID, TX::getText(TX::txtAlready), attributeIdx);
        return false;
    }
}

void StorageAdministrator::storePeerConnectionDetails(
        const uint8_t *pMacAddress, uint16_t networkAddress,
        uint16_t attributeIdx, uint8_t outgoingRoute, uint8_t linkStatus,
        bool entryInvalid) {
    static uint8_t con_data[12];
    memcpy(&con_data[ciOfsMacAdr], pMacAddress, 8);
    uint8Operations::insertUint16intoUint8Array(&con_data[ciOfsNwAdr], networkAddress);
    con_data[ciOfsOutgoingRoute] = outgoingRoute;
    con_data[ciOfsLinkStatus] = linkStatus;
    IKeyContainerStorage::container attributeContainer;
    attributeContainer.data = con_data;
    attributeContainer.length = sizeof(con_data);
    IKeyContainerStorage::key_flags kf;
    kf.all = 0;
    kf.bit.invalid = entryInvalid;
    if (storeIfNewContainer(clientInfoTable, kf.all, attributeIdx,
            eapClientInfoTable, attributeContainer)) {
        // ToDo(AME) /optional? may be not required
        // // Set a container to invalid, if he has the same content, but us another attributeIdx
        // IKeyContainerStorage::skey temp_key, load_key;
        // temp_key.extended_key.key    = clientInfoTable;
        // temp_key.extended_key.index  = attributeIdx;
        // temp_key.extended_key.flag.all = 0;
        // load_key = temp_key;
        // for (uint32_t i=1;i<=networkMemberMax;i++) {
        //
        // }
    }
}

void StorageAdministrator::StoreBroadcastGroups(uint8_t mapOffset, int count) {
    IRoutingManager *pRM = ProgramConfig::getRoutingManagerPtr();
    uint8_t con_data[CircularBufferManager::MaxDataLen];
    uint8_t *pcd;
    int16_t conTupelCount;
    uint16_t conIdx, conNum;
    IKeyContainerStorage::container BCContainer;
    BCContainer.data = con_data;
    IKeyContainerStorage::key_flags kf;
    kf.all = 0;
    kf.bit.invalid = false;
    memset(con_data, 0, CircularBufferManager::MaxDataLen);
    conNum = 0;
    while (count > 0) {
        pcd = &con_data[1];
        conIdx = 2;
        conTupelCount = 0;
        for (int i = 0; i < count; i++) {
            IRoutingManager::MemberIpTupel tupel;
            pRM->storeData(IRoutingManager::routingRowEntryIndex, (uint8_t) (i + mapOffset));
            tupel.ip = pRM->getData(IRoutingManager::GET_IP_AT_INDEX);
            tupel.meberID = (uint8_t) pRM->getData(IRoutingManager::GET_MEMBER_ID_AT_INDEX);
            *pcd++ = tupel.meberID;
            uint8Operations::insertUint16intoUint8Array(pcd, tupel.ip);
            pcd += 2;
            conTupelCount++;
            conIdx = (uint16_t) (conIdx + 3);
            if (conIdx + 3 > CircularBufferManager::MaxDataLen) {
                break;
            }
        }
        BCContainer.length = (uint8_t) conIdx;
        count -= conTupelCount;
        mapOffset = (uint8_t) (mapOffset + conTupelCount);
        con_data[0] = (uint8_t) conTupelCount;
        storeIfNewContainer(broadcastGroupTable, kf.all, conNum, 0, BCContainer);
        conNum++;
    }
}

// @return: number of specified tuples
uint8_t StorageAdministrator::RestoreBroadCastGroups() {
    uint16_t conNum = 0;
    uint8_t count, mapOffset = 0;
    uint8_t result = 0;
    uint8_t *pbcgt;
    IRoutingManager::MemberIpTupel tupel;
    IRoutingManager *Rm = ProgramConfig::getRoutingManagerPtr();
    IKeyContainerStorage::skey load_key;
    load_key.extended_key.key = broadcastGroupTable;
    while (conNum < 2) {
        load_key.extended_key.index = conNum;
        load_key.extended_key.flag.all = 0;
        IKeyContainerStorage::container loadedCon;
        storageInterfacePtr->loadContainerWithKey(&load_key, &loadedCon);
        if (loadedCon.data) {
            pbcgt = (uint8_t*) loadedCon.data;
            count = *pbcgt++;
            if ((count + mapOffset) < IRoutingManager::maxTupelAmount) {
                for (int i = 0; i < count; i++) {
                    tupel.meberID = *pbcgt++;
                    tupel.ip = uint8Operations::sum2BytesFromLSB(pbcgt);
                    pbcgt += 2;
                    Rm->storeData(IRoutingManager::routingRowEntryIndex,
                            (uint8_t) (i + mapOffset));
                    Rm->storeData(IRoutingManager::GET_IP_AT_INDEX, tupel.ip);
                    Rm->storeData(IRoutingManager::GET_MEMBER_ID_AT_INDEX,
                            (uint8_t) tupel.meberID);
                    result++;
                }
                mapOffset = (uint8_t) (mapOffset + count);
                conNum++;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return result;
}

char *StorageAdministrator::getStatusInfo() {
    // TODO(AME) Optimize return of statusText
    // Idea:
    // - in storePlcGetConfirmationFromLayer store the variable attributeID, attributeIdx
    //   and the compare result of compare2Container in static variable.
    // - At this point, create the text again with the saved variables,
    //   but use the Global2-Buffer as a return container
    return statusText;
}

StorageAdministrator::~StorageAdministrator() = default;

}  // namespace AME_SRC

