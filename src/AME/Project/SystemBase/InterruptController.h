/*
 * InterruptController.h
 *
 *  Created on: Jun 5, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 *
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_INTERRUPTCONTROLLER_H_
#define SRC_AME_PROJECT_SYSTEMBASE_INTERRUPTCONTROLLER_H_

namespace AME_SRC {

class InterruptController {
 public:
    InterruptController();
    virtual ~InterruptController();

    enum eIntSource {
        none = 0,             // no interrupts enabled
        plcModem = 0x01,      // Modem Interrupts
        terminal = 0x02,      // Serial Terminal Device
        i2c = 0x04,           // i2c Bus
        cmt0 = 0x08,          // osTimer
        cmt1 = 0x10,          // BootLoader
                              // ToDo (AME): Add handler for AD-Conversion Interrupts #PossibleBug
        all = plcModem | terminal | i2c | cmt0 | cmt1
    };

    static void enable(eIntSource mask);
    static void disable(eIntSource mask);

 private:
    static eIntSource currentMask;
};


}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_INTERRUPTCONTROLLER_H_
