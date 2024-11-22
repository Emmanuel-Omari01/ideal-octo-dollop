/*
 * StorageAdministrator.h
 *
 *  Created on: 28.02.2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMLOGIC_STORAGEADMINISTRATOR_H_
#define SRC_AME_PROJECT_SYSTEMLOGIC_STORAGEADMINISTRATOR_H_

#include "../StorageManagement/IKeyContainerStorage.h"
#include "../PlcCommunication/G3PlcSystem/PlcCommand.h"
#include "../PlcCommunication/PlcCommandTypeSpecification.h"
#include "../PlcCommunication/AliveHandling/MessageForwarding/IRoutingManager.h"

namespace AME_SRC {

class StorageAdministrator {
 public:
    enum storageKeys {  // Container size of data in Bytes | amount of containers (10 network members)
        adpRouteTable = 0x80,                       //  24 | 10
        // macFrameCounter = 0x81,  DS: not used //
        macNeighbourTable = 0x82,                   //  32 | 10
        clientInfoTable = 0x83,                     //  24 | 10
        broadcastGroupTable = 0x84,                 //  32 | 1
        sysParameter = 0,                           //  24 | 1
        frameCounter                                //  8  | 1
    };
    enum clientInfoTableOffsets {
        ciOfsMacAdr = 0,
        ciOfsNwAdr = 8,
        ciOfsOutgoingRoute = 10,
        ciOfsLinkStatus = 11
    };

 private:
    static IKeyContainerStorage *storageInterfacePtr;
    static bool saveInfoAgainRequest;  // True = save container data again (after deletion of an data flash block)
//  static uint8_t attributeContainerLength;

    static char statusText[50];

    static bool isPlcGetAttributeIdStorable(const PlcCommand& confirmation, LAYER layer,
                                            uint16_t attributeId, uint16_t *attributeIdx,
                                            const uint8_t** pData, uint8_t *data_len,
                                            enum StorageAdministrator::storageKeys* skey);

 public:
    StorageAdministrator();

    static void storePlcGetConfirmationFromLayer(const PlcCommand &confirmation, LAYER layer);
    static bool storeIfNewContainer(enum storageKeys store_key,
                                    uint8_t store_flags, uint16_t attributeIdx, uint16_t attributeID,
                                    IKeyContainerStorage::container attributeContainer);
    static void storePeerConnectionDetails(const uint8_t *pMacAddress,
                                           uint16_t networkAddress, uint16_t attributeIdx,
                                           uint8_t outgoingRoute, uint8_t linkStatus, bool entryInvalid);

    static char* getStatusInfo();
    virtual ~StorageAdministrator();
    static IKeyContainerStorage* getStorageInterfacePtr() {
        return storageInterfacePtr;
    }
    static bool getSaveAgainRequest() {  // save container with table information again?
        return saveInfoAgainRequest;
    }
    static void releaseSaveAgainRequest() {
        saveInfoAgainRequest = false;
    }

    static void StoreBroadcastGroups(uint8_t mapOffset, int count);
    static uint8_t RestoreBroadCastGroups();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMLOGIC_STORAGEADMINISTRATOR_H_

