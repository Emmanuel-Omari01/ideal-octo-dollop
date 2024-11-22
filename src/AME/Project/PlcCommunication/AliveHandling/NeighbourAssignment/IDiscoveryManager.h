/*
 * IDiscoveryManager.h
 *
 *  Created on: 25.05.2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IDISCOVERYMANAGER_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IDISCOVERYMANAGER_H_

#include <cstdint>

namespace AME_SRC {

class IDiscoveryManager {  // @suppress("Class has a virtual method and non-virtual destructor")
 public:
    enum eDiscoveryState {
        idleDiscovery, discovery, showDiscovery, done
    };
    // IDiscoveryManager();
    virtual uint8_t getData(const char *cWhat) = 0;
    virtual char* showData(const char *cWhat) = 0;
    virtual bool isDataShown() = 0;
    virtual void storeData(uint8_t status, const uint8_t *data, uint8_t length) = 0;
    virtual char* triggerDiscovery(uint8_t select, const char *cParameter) = 0;
    // virtual bool isDataValid() = 0;
    // virtual ~IDiscoveryManager();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IDISCOVERYMANAGER_H_
