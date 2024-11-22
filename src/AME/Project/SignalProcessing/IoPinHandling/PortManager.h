/*
 * PortManager.h
 *
 *  Created on: 19.01.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_PORTMANAGER_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_PORTMANAGER_H_

#include <stdint.h>

namespace AME_SRC {

class PortManager {
 private:
    static bool isNotInstanciated;
    static PortManager *singlePtr;
    PortManager();

 public:
    // ToDo(AME): create a version which needs less RAM memory #MemoryOptimization
    typedef enum {
        Pin0 = 0, Pin1, Pin2, Pin3, Pin4, Pin5, Pin6, Pin7, NoPin
    } pinPosition;
    typedef enum {
        Input = 0, Output
    } pinDirection;
    typedef enum {
        GeneralIOPort = 0, PeriphalFunction
    } pinMode;
    typedef enum {
        Low = 0, High
    } pinLevel;
    typedef enum {
        Port0 = 0,
        Port1 = 1,
        Port2,
        Port3,
        Port4,
        PortA,
        PortB,
        PortC,
        PortE,
        NoPort
    } portName;
    ~PortManager();
    void setPinDirection(portName port, pinPosition pinPosition,
            pinDirection direction);
    void setPinMode(portName port, pinPosition pinPosition, pinMode mode);
    void setPinLevel(portName port, pinPosition pinPosition, pinLevel mode);
    void setPinFunctionByte(portName port, pinPosition pinPosition,
            uint8_t byte);
    void setPortSwitchingRegister(portName port, pinPosition pinPosition);
    pinLevel getPinLevel(portName port, pinPosition pinPosition);
    static PortManager* getInstance();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_IOPINHANDLING_PORTMANAGER_H_
