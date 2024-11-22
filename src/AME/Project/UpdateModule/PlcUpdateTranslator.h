/* Copyright (C) 2024 Tobias Hirsch - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the XYZ license.
 */

#ifndef SRC_AME_PROJECT_UPDATEMODULE_PLCUPDATETRANSLATOR_H_
#define SRC_AME_PROJECT_UPDATEMODULE_PLCUPDATETRANSLATOR_H_

#include "IUpdateTranslator.h"
#include "../HelpStructures/ArrayQueue.h"

namespace AME_SRC {

class PlcUpdateTranslator: public IUpdateTranslator {
 public:
    PlcUpdateTranslator();
    virtual ~PlcUpdateTranslator();
    bool isUpdateMessage() override;
    UpdateMessage& translateMessage(const char *message) override;
    void storeMessage(const UpdateMessage& uMessage_) override;
    UpdateMessage& getNextTranslation() override;

    const char getUpdatePrefix() {
        return _kUpdatePrefix;
    }

 private:
    ArrayQueue<UpdateMessage, 10> updateQueue;
    UpdateMessage _uMessage;
    bool _isMessageValid;
    const char _kUpdatePrefix = 'u';
};


}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_UPDATEMODULE_PLCUPDATETRANSLATOR_H_
