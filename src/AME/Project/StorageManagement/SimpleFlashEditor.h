/*
 * simpleFlashEditor.h
 *
 *  Created on: 21.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_STORAGEMANAGEMENT_SIMPLEFLASHEDITOR_H_
#define SRC_AME_PROJECT_STORAGEMANAGEMENT_SIMPLEFLASHEDITOR_H_

#include <cstdint>

#include "../SystemBase/Global.h"
#include "../StorageManagement/IKeyContainerStorage.h"
#include "IStorage.h"
#include "RenFlashApiAdapter.h"

namespace AME_SRC {

class SimpleFlashEditor: public IStorage {
 public:
    typedef struct {    // structure for process parameters
                        // Offset (hex)
        uint16_t board_ID;              // 00 Module no
        uint16_t autoStartFlowMode;     // 02 Bitmask, see AutoStartMode
        uint16_t targetNetId;           // 04 panID for force join
        uint16_t ownNetworkAddress;     // 06 2nd parameter for the force join (lbaAddress?)
        uint16_t fjCallouts;            // 08 callouts for force join (coordinator)
        uint16_t fjScale;               // 0A scaling factor for delay, force join (coordinator)
        uint8_t fjDelay[3];             // 0C..E dwell for force join (coordinator)
    } Parameter_A;                      // memory requirement: actual 0Eh Bytes/Record
    typedef struct {
        // Offset (hex)
        uint32_t FrameCounter;          // 00 telegram No
    } Parameter_FC;                     // memory requirement: actual 04h Bytes/Record
    typedef struct {
        uint16_t R_dest_Addr;           // 00 Address of the destination
        uint16_t R_next_Addr;           // 02 Address of the next hop on the route towards the destination
        uint16_t R_metric;              // 04 Cumulative link cost along the route towards the destination
        uint8_t R_hop_count;            // 06 Number of hops of the selected route to the destination
        uint8_t R_weak_link_count;      // 07 Number of weak link to destination. It ranges from 0 to adpMaxHops
        uint16_t validTime;             // 08 The remaining time to live of this entry in the broadcast
                                        //    log table [minutes]
    } Parameter_RT;                     // memory requirement: actual 0Ah Bytes/Record

    enum AutoStartMode {
        SearchMode = 0,                     // Bit 0 cleared => default search
        ForceJoinMode = 1,
        ForceJoinWithVerification = 2,      // Bit 1
        ForceJoinWithTimeOut = 4            // Bit 2
    };
    enum EFJDelayIdx {
        fjDelayX, fjDelayY, fjDelayZ
    };  // indexes for Parameter_A.fjDelay[]

    void store(uint8_t *data, uint32_t adress, uint16_t size);

    SimpleFlashEditor();
    virtual ~SimpleFlashEditor();
//  static bool writeFlashData(uint8_t block_nr);
    static bool readFlashData();
    static uint32_t getFrameCounter();
    static void setFrameCounter(uint32_t);
    static void initFlash();
    static char* eraseDataFlash(char*);

    static bool readParamFlash();
    static bool readFCFlash();
    static bool readFlashContainer(int key, void *dest, uint32_t destLen);
    static bool writeFCFlash();
    static Parameter_A& getFlashA() {
        return flash_A;
    }
    static const Parameter_FC& getFlashFC() {
        return flash_FC;
    }

    static void setFlashA(Parameter_A &flashA) {
        flash_A = flashA;
    }

    static void preSetFlashA(bool withID) {
        flash_A.autoStartFlowMode = SearchMode;
        if (withID) {
            flash_A.board_ID = 0;
        }
        flash_A.fjCallouts = 3;
        flash_A.fjDelay[fjDelayX] = 5;
        flash_A.fjDelay[fjDelayY] = 60;
        flash_A.fjDelay[fjDelayZ] = 5;
        flash_A.fjScale = 2;
        flash_A.ownNetworkAddress = 0;
        flash_A.targetNetId = 0;
    }
    static bool isForceJoinMode() {
        return (flash_A.autoStartFlowMode & ForceJoinMode) != 0;
    }

    static void setFlashFC(Parameter_FC &pFlash) {
        flash_FC = pFlash;
    }
    static uint16_t getBoardID() {
        return flash_A.board_ID;
    }
    static uint16_t getFJCallout() {
        return flash_A.fjCallouts;
    }
    static uint8_t getFJDelay(uint16_t index) {
        return flash_A.fjDelay[index];
    }
    static uint16_t getFJScale() {
        return flash_A.fjScale;
    }
    static void setSysParamID(uint16_t id);
    static void setAutoStartFlowMode(uint16_t mode);
    static uint16_t getAutoStartFlowMode() {
        return flash_A.autoStartFlowMode;
    }
    static void setTargetNetId(uint16_t net_id);
    static void setOwnNetworkAddress(uint16_t adr);
    static void updateSysParamFlash();
    static void setFJCallout(uint16_t ca);
    static void setFJDelay(uint8_t delay, uint16_t index);
    static void setFJScale(uint16_t scale);
    static char* memoryWriteTest(char *value);
    static char* memoryReadTest(char *value);
    static char* setDefaultSysParameter(char *dummy);

 private:
    static Parameter_A flash_A;
    static Parameter_FC flash_FC;
//  static Parameter_RT flash_RT;

    static flash_err_t Erase_FlashData(U32 user_block);
};


}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_STORAGEMANAGEMENT_SIMPLEFLASHEDITOR_H_
