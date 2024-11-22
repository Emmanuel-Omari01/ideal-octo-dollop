/*
 * broadcastRoutingManager.h
 *
 *  Created on: Jul 14, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_BROADCASTROUTINGMANAGER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_BROADCASTROUTINGMANAGER_H_

#include "IRoutingManager.h"

namespace AME_SRC {

class BroadcastRoutingManager :public IRoutingManager{
 public:
    BroadcastRoutingManager();
    uint16_t getData(enum IRoutingManager::dataKey);
    char* showData(const char *cWhat);
    bool isDataShown();
    void storeData(enum dataKey, uint16_t value);
    virtual ~BroadcastRoutingManager();

 private:
    uint8_t storeIndex_;    // index for a tuple
    uint8_t searchValue_;
    MemberIpTupel memberIpMap_[maxTupelAmount];
    enum {Type_MemberID, Type_MemberIP};
    IRoutingManager::MemberIpTupel* searchForValueByIdOrIp(uint16_t value, bool type);
    uint16_t getIpFromSearchValueWithOffset(int offset);
    uint16_t getIpForSearchRequest(enum IRoutingManager::dataKey dtaKey);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_BROADCASTROUTINGMANAGER_H_
