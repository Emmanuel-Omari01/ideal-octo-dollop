/*
 * SimpleTimeOutWatcher.h
 *
 *  Created on: 02.06.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_SIMPLETIMEOUTWATCHER_H_
#define SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_SIMPLETIMEOUTWATCHER_H_

#include "ITimeOutWatcher.h"

#include "../../HelpStructures/BitMask.h"

namespace AME_SRC {

class SimpleTimeOutWatcher: public ITimeOutWatcher {
 public:
    SimpleTimeOutWatcher();
    virtual ~SimpleTimeOutWatcher();

    void start(uint8_t id) override;
    void stopAndReset(uint8_t id) override;
    char* getStatus() override;
    void update() override;
    void registerTimeout(timeOutInfo &info) override;
    void clearTimeout(uint8_t index) override;
    bool editTimeoutdelayForID(uint8_t searchedID, uint8_t newDelay) override;
    void resetDelayForIndex(uint8_t index) override;
    ITimeOutWatcher::timeOutInfo* findTimeOutWithID(uint8_t searchedID) override;

 private:
    timeOutInfo timeOutContainers[16];  // TODO(AME): dynamic version

    void setTimeOutInfoAtID(uint8_t id, int8_t timeOuts, int8_t callOuts,
            int8_t timeOutAmount, int8_t callOut, uint16_t delay,
            void (*timeOutFunc)(), void (*callOutFunc)(), bool started,
            uint8_t searchedID);
    void updateTimeOutInfo(timeOutInfo &info);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_SIMPLETIMEOUTWATCHER_H_
