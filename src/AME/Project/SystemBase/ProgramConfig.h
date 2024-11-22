/*
 * ProgramConfig.h
 *
 *  Created on: 21.01.2021
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_PROGRAMCONFIG_H_
#define SRC_AME_PROJECT_SYSTEMBASE_PROGRAMCONFIG_H_

#include <stdlib.h>
#include <stdio.h>

#include "PCBPortSelector.h"
#include "WatchDog/WatchDog.h"
#include "../SignalProcessing/IoPinHandling/IoPin.h"
#include "../Terminal/CommandInterpreter.h"
#include "../Terminal/TermContextFilter.h"
#include "../StorageManagement/FlashMemoryN25Q.h"
#include "../SignalProcessing/Aktorik/AdditionalOutputDriver.h"
#include "../SignalProcessing/Sensorik/Multiplexor.h"
#include "../SignalProcessing/IoPinHandling/SwitchMatrixAdmin.h"
#include "../SystemLogic/AutostartFlow.h"
#include "../StorageManagement/CircularBufferManager.h"
#include "../CompositeComponents/FoundationModule/ITimeOutWatcher.h"
#include "../PlcCommunication/AliveHandling/NeighbourAssignment/AdjazenzParent.h"
#include "../PlcCommunication/AliveHandling/MessageForwarding/ForwardingFlow.h"
#include "../PlcCommunication/AliveHandling/MessageForwarding/BGroupForwarding.h"
#include "../PlcCommunication/AliveHandling/MessageForwarding/BroadcastRoutingManager.h"
#include "../PlcCommunication/AliveHandling/MessageForwarding/JumpMapper.h"
#include "../SignalProcessing/Aktorik/LED/LEDSignalFlow.h"
#include "../SignalProcessing/Sensorik/MuxIoPin.h"
#include "../Test/UnitSuite/UnitSuiteTests.h"
#include "../Test/Integration/IntegrationsTests.h"
#include "../UpdateModule/SoftwareDeploymentFlow.h"

extern "C" void spiTest(uint32_t deviceSelect);
extern "C" void spi_flash_test(void);

namespace AME_SRC {

class ProgramConfig {
// NO format change of define, will be used in build process to identify software and hardware compatibility
// search for string "#define HW_VERSION ? //" where ? is the used version, see LibConfig.sh
#define HW_VERSION 12  // Hardware Version: 0=Evaluation board, 1=1. limited lot production "Kleinserie"
                       // 12 => Version for V1-A2.a - B2.a

#if HW_VERSION >= 1
#define CPX_BOOT0_MODE 1  // 0=SROM-Boot, 1=UART-Boot
#endif

#define ALIVE_PREFER_BROADCAST_TRANSMISSION 1  // if defined then the transmission of PLC telegrams within the
                                               // alive is handled by broadcast telegrams

 private:
    static int plcBoardID;
    static bool coordinatorStatus;

    static SerialProfile *plcProfile_;
    static AutostartFlow *autostartFlow;
    static AssignmentFlow *assignmentFlow_;
    static CommandInterpreter terminal;
    static ModemInterpreter *plcModemPtr;
    static SwitchMatrixAdmin *switchMatrixAdminPtr;
    static TimeMeasurement timeMeas;
    static TimeMeasurement::measureJob *runTimeJobPtr;     // Variable for runtime of the main process
    static TimeMeasurement::measureJob *aliveTimeJobPtr;   // Timing of the entire alive process
    static TimeMeasurement::measureJob *aliveRoundJobPtr;  // Timing of a single alive round trip
    static ITimeOutWatcher *simpleTimeOutWatcherPtr;
    static BroadcastRoutingManager *routingManagerPtr;
    static JumpMapper *aliveJumpMapper_;
    static LEDSignalFlow* ledFlow_;
    static SoftwareDeploymentFlow* _deployFlow;
    static IoCommandManager *ioCmdManger_;
    static SerialDriver *terminalDriverPtr;
    static SerialDriver modemDrive;
    static TermContextFilter *termFilterPtr_;
    static FlashMemoryN25Q *FlashMemoryN25Q_;

    static WatchDog watchDog;
    static bool isFirstInit50ms;
    static bool memoryProcessing;
    static bool blockStoreFlashRouteTable;
    static bool isTestAutomation_;
    static void light10TimesLedIdCodeAndResetId(uint8_t id);
    static void showLedIdCode(uint8_t id);
    static void storeFrameCounter();
    void pcbInit();

 public:
    static BGroupForwarding *groupForwardingPtr;
    static ForwardingFlow *aliveForwardingFlow_;
    static UnitSuiteTests unitSuiteTests;
    static IntegrationsTests integrationsTest;

    enum timeOutIndex {
        rBroadcastGroupTOutID = 1,
        rAliveTOutID = 2,
        assignmentFlowTOutID = 3,
        routeDiscoveryTOutID = 4,
        outerAssignmentFlowTOutID = 5,
        forceJoinAliveTOutID = 6,
        simpleShortTOutID = 7,
        aliveACKLastTripID = 8,
        /*Alive Jump*/
        aliveNoResponsReportTOutID = 9,
        aliveJumpRequestTOutID = 10,
        rAliveJumpTOutID = 11,
        /*Alive Jump*/
        cmdRetransmittTOutID = 12,
        roundtripMonitorTOutID = 13,
        aliveACKReplyOnceID = 14,
        i2cTaskID = 15,
    };
    enum eAliveLEDMode {
        joiningPhase, aliveRunning, forceJoinTimeout
    };

    ProgramConfig();
    virtual ~ProgramConfig();
    static void window100micro();
    static void window1ms();
    static void window5ms();
    static void window50ms();
    static void window1s();
    static void window60s();

    static void bootWindow50ms();
    static void bootWindow100micro();

    static PCBPortSelector* pcbPortSelector_;


    friend void plcModemStartCommand();

    static void setPLCProfileForModemDriver();

    static int getPlcBoardId() {
        return plcBoardID;
    }

    static bool isCoordinator() {
        return coordinatorStatus;
    }

    static void setCoordinatorStatus(bool status) {
        coordinatorStatus = status;
    }

    static TimeMeasurement::measureJob* getRunTimeJobPtr() {
        return runTimeJobPtr;
    }

    static void setRunTimeJobPtr(TimeMeasurement::measureJob *ptr) {
        runTimeJobPtr = ptr;
    }
    static void setBlockStoreFlashTable(bool lSet) {
        blockStoreFlashRouteTable = lSet;
    }
    static bool getBlockStoreFlashTable() {
        return blockStoreFlashRouteTable;
    }

    static void doSendGlobalReset();

    static TimeMeasurement::measureJob* getAliveTimeJobPtr() {
        return aliveTimeJobPtr;
    }

    static void setAliveTimeJobPtr(TimeMeasurement::measureJob *ptr) {
        aliveTimeJobPtr = ptr;
    }

    static TimeMeasurement::measureJob* getAliveRoundJobPtr() {
        return aliveRoundJobPtr;
    }

    static void setAliveRoundJobPtr(TimeMeasurement::measureJob *ptr) {
        aliveRoundJobPtr = ptr;
    }

    static ModemInterpreter* getPlcModemPtr() {
        return plcModemPtr;
    }
    static UnitSuiteTests* getUnitSuiteTestsPtr() {
        return &unitSuiteTests;
    }
    static IntegrationsTests* getIntegrationTestPtr() {
        return &integrationsTest;
    }

    static ITimeOutWatcher* getTimeOutWatcherPtr() {
        return simpleTimeOutWatcherPtr;
    }
    static SerialDriver * getTerminalDriverPtr() {
        return terminalDriverPtr;
    }
    static CommandInterpreter* getTerminal() {
        return &terminal;
    }

    static TermContextFilter* getTerminalFilter() {
        return termFilterPtr_;
    }

    static FlashMemoryN25Q* getFlashMemoryDriverPtr() {
        return FlashMemoryN25Q_;
    }

    static BroadcastRoutingManager* getRoutingManagerPtr() {
        return routingManagerPtr;
    }

    static LEDSignalFlow& getLedFlow() {
        return *ledFlow_;
    }

    static AutostartFlow& getAutostartFlow() {
        return *autostartFlow;
    }

    static PCBPortSelector* getPcbPortSelector() {
        return pcbPortSelector_;
    }

    static SerialDriver getModemDrive() {
        return modemDrive;
    }

    void setModemDrive(SerialDriver modemDrive) {
        this->modemDrive = modemDrive;
    }
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_PROGRAMCONFIG_H_

