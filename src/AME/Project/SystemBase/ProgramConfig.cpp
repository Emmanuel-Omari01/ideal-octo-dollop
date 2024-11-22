/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
   	Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
	Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
	gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

/***********************************************************************************************************************
* File Name     : ProgramConfig.cpp
* Tool-Chain    : CCRX, GCC
* Device        : R5F51118ADFL 48-pin: 512 kByte ROM, 64 KByte RAM, 8 KByte Data Flash @ 32MHz
* Authors       : Tobias Hirsch, Detlef Schulz, AME
* Description   : This module contains the program configuration of the user application, including the assignment
*                 the time loops to specific processes as well as the control of relevant modules and interfaces
***********************************************************************************************************************/

// Modules
#include "ProgramConfig.h"
// User
#include "../Terminal/Sci/SerialComInterface.h"
#include "../Terminal/SerialProfiles/SerialStandardProfile.h"
#include "../Terminal/SerialProfiles/SerialPlcProfile.h"
#include "../Terminal/SerialProfiles/SerialCpxBootProfile.h"
#include "../SignalProcessing/AnalogPinHandling/AnalogPin.h"
#include "../SignalProcessing/I2C/I2cApp.h"
#include "../PlcCommunication/PlcTerminalCommandConnector.h"
#include "../PlcCommunication/AliveHandling/NeighbourAssignment/SimpleDiscoveryManager.h"
#include "../CompositeComponents/FoundationModule/SimpleRandGenerator.h"
#include "MicrocontrollerConfig.h"
#include "../HelpStructures/CharOperations.h"
#include "../StorageManagement/SimpleFlashEditor.h"
#include "../Terminal/CommandTabProvider.h"
#include "../UpdateModule/CodeFlashEditor.h"
#include "../Terminal/FlashAccessHelper.h"
#include "../UpdateModule/UpdateTermConnector.h"
#include "ConfigTermConnector.h"
#include "../SystemLogic/IEventStateProvider.h"
#include "../SystemLogic/SystemStateController.h"


namespace AME_SRC {

// Initialization of the serial interfaces to the modem and to the external receiver
#define CONFIG_UART_INIT
#ifdef CONFIG_UART_INIT
SerialProfile* serialProfilePtr = new SerialStandardProfile();
SerialCpxBootProfile* cpxProfilePtr = new SerialCpxBootProfile(ProgramConfig::getPlcModemPtr());
SerialProfile* bootProfilePtr = cpxProfilePtr;


static SerialDriver termDrive = SerialDriver(SerialComInterface::getInstance(sci12, baud115200, *serialProfilePtr));
#if IS_FIRMWARE_DOWNLOAD_ENABLED < 1
static SerialDriver modemDrive = SerialDriver(SerialComInterface::getInstance(sci5, baud115200, &plcProfile));
#else
static SerialComInterface* modemSCI = SerialComInterface::getInstance(sci5, baud115200, *bootProfilePtr);
SerialDriver ProgramConfig::modemDrive = SerialDriver(modemSCI);
#endif

PCBPortSelector *ProgramConfig::pcbPortSelector_ = NULL;
/***
#if HW_VERSION < 1
  static IoPin onboardLED1 = IoPin(PortManager::PortB, PortManager::Pin5, PortManager::Output, Pin::active);  // P17->PB5
  static IoPin onboardLED2 = IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output, Pin::active);  // PA1
#elif HW_VERSION == 1
  static IoPin onboardLED1 = IoPin(PortManager::Port0, PortManager::Pin3, PortManager::Output, Pin::active);  // P03
  static IoPin onboardLED2 = IoPin(PortManager::Port4, PortManager::Pin1, PortManager::Output, Pin::active);  // P41
  static IoPin onboardLED3 = IoPin(PortManager::Port4, PortManager::Pin2, PortManager::Output, Pin::active);  // P42
#elif HW_VERSION == 12
  static IoPin onboardLED1 = IoPin(PortManager::PortC, PortManager::Pin1, PortManager::Output, Pin::aktiv);  // PC1
  static IoPin onboardLED2 = IoPin(PortManager::PortC, PortManager::Pin0, PortManager::Output, Pin::aktiv);  // PC0
  static IoPin onboardLED3 = IoPin(PortManager::PortB, PortManager::Pin5, PortManager::Output, Pin::aktiv);  // PB5
#endif
LEDSignalFlow ProgramConfig::ledFlow_ = LEDSignalFlow(onboardLED1, onboardLED2);
***/
LEDSignalFlow *ProgramConfig::ledFlow_ = NULL;
SoftwareDeploymentFlow* ProgramConfig::_deployFlow = NULL;

BroadcastRoutingManager* routingManager = new BroadcastRoutingManager();
BroadcastRoutingManager* ProgramConfig::routingManagerPtr = routingManager;

SerialDriver* ProgramConfig::terminalDriverPtr = &termDrive;
TermContextFilter *ProgramConfig::termFilterPtr_ = NULL;

// AssignmentFlow &assignmentFlowRef = *(new AssignmentFlow(termDrive, ProgramConfig::getLedFlow()));
AssignmentFlow *ProgramConfig::assignmentFlow_ = NULL;
ITimeOutWatcher* ProgramConfig::simpleTimeOutWatcherPtr = new SimpleTimeOutWatcher();
// JumpMapper ProgramConfig::aliveJumpMapper_ = JumpMapper(simpleTimeOutWatcherPtr, &termDrive, assignmentFlowRef);
JumpMapper *ProgramConfig::aliveJumpMapper_ = NULL;
BGroupForwarding* ProgramConfig::groupForwardingPtr = NULL;
// ForwardingFlow &ProgramConfig::aliveForwardingFlow = *(new ForwardingFlow(groupForwardingPtr, ioCmdManger_,
//                                                                          *simpleTimeOutWatcherPtr, &termDrive));
ForwardingFlow *ProgramConfig::aliveForwardingFlow_;

// static SerialProfile* plcProfile = new SerialPlcProfile(ProgramConfig::aliveForwardingFlow);
SerialProfile *ProgramConfig::plcProfile_ = NULL;
FlashMemoryN25Q *ProgramConfig::FlashMemoryN25Q_ = NULL;
#endif

// Initialization of the IO interfaces to manage external inputs and outputs
#define CONFIG_IO_INIT
#ifdef CONFIG_IO_INIT
#if (HW_VERSION == 12)
  // LED see ledFlow_

  /***
  AnalogPin cpu_Analog1 = AnalogPin(PortManager::PortE, PortManager::Pin5, PortManager::Input);     // PE5

  static IoPin CP3_Boot0 = IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output);       // PA1

  static IoPin FCC_CENELEC_IN = IoPin(PortManager::PortC, PortManager::Pin3, PortManager::Input);    // PC3

  static IoPin out1 = IoPin(PortManager::Port0, PortManager::Pin3, PortManager::Output);   // P03 motor 1 Relay
  static IoPin out2 = IoPin(PortManager::Port0, PortManager::Pin5, PortManager::Output);   // P05 motor 2 Relay
  static IoPinGroup moduleOutputs = IoPinGroup(2, &out1, &out2);

  static IoPin in1 = IoPin(PortManager::PortE, PortManager::Pin7, PortManager::Input);  // PE7 Input 1
  static IoPin in2 = IoPin(PortManager::Port4, PortManager::Pin6, PortManager::Input);  // P46
  static IoPin in3 = IoPin(PortManager::PortE, PortManager::Pin6, PortManager::Input);  // PE6
  static IoPin in4 = IoPin(PortManager::Port4, PortManager::Pin4, PortManager::Input);  // P44
  static IoPinGroup moduleInputs = IoPinGroup(4, &in1, &in2, &in3, &in4);

  IoPin* cpxResetPin = new IoPin(PortManager::PortE, PortManager::Pin3, PortManager::Output);  // PE3

  // Initialize analog pins
  static AnalogPin Version_A1 = AnalogPin(PortManager::Port4, PortManager::Pin1, PortManager::Input);   // P41
  static AnalogPin Version_A2 = AnalogPin(PortManager::Port4, PortManager::Pin2, PortManager::Input);   // P42
  static AnalogPin Version_B1 = AnalogPin(PortManager::Port4, PortManager::Pin3, PortManager::Input);   // P43
  static AnalogPin Version_B2 = AnalogPin(PortManager::PortE, PortManager::Pin4, PortManager::Input);   // PE4
  ***/

#elif (HW_VERSION == 1)
  /***
  static IoPin plcJoinLED = IoPin(PortManager::Port0, PortManager::Pin3, PortManager::Output);  // P03
  static IoPin plcLED2 = IoPin(PortManager::Port4, PortManager::Pin1, PortManager::Output);     // P41
  static IoPin plcLED3 = IoPin(PortManager::Port4, PortManager::Pin2, PortManager::Output);     // P42

  AnalogPin cpu_Analog1 = AnalogPin(PortManager::Port4, PortManager::Pin0, PortManager::Input);     // P40
  static IoPin CP3_Boot0 = IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output);       // PA1
  static IoPin FCC_CENELEC_A1 = IoPin(PortManager::PortC, PortManager::Pin0, PortManager::Output);   // PC0
  static IoPin FCC_CENELEC_A2 = IoPin(PortManager::PortC, PortManager::Pin1, PortManager::Output);   // PC1
  static IoPin BackUp = IoPin(PortManager::PortE, PortManager::Pin3, PortManager::Output);          // PE3

  static IoPin out1 = IoPin(PortManager::PortE, PortManager::Pin7, PortManager::Output);   // PE7 motor 1 Relay
  static IoPin out2 = IoPin(PortManager::PortE, PortManager::Pin0, PortManager::Output);   // PE0 motor 2 Relay
  static IoPinGroup moduleOutputs = IoPinGroup(2, &out1, &out2);
  static IoPin in1 = IoPin(PortManager::Port4, PortManager::Pin6, PortManager::Input);  // P46 Input 1
  static IoPin in2 = IoPin(PortManager::PortE, PortManager::Pin6, PortManager::Input);  // PE6
  static IoPin in3 = IoPin(PortManager::Port4, PortManager::Pin3, PortManager::Input);  // P43
  static IoPin in4 = IoPin(PortManager::Port4, PortManager::Pin4, PortManager::Input);  // P44
  static IoPinGroup moduleInputs = IoPinGroup(4, &in1, &in2, &in3, &in4);

  IoPin* cpxResetPin = new IoPin(PortManager::PortA, PortManager::Pin0, PortManager::Output);  // PA0
  ***/
#else
  /***
  IoPin mux0 = IoPin(PortManager::PortC, PortManager::Pin5, PortManager::Output);          // PC5
  IoPin mux1 = IoPin(PortManager::PortC, PortManager::Pin6, PortManager::Output);          // PC6
  IoPin mux2 = IoPin(PortManager::PortC, PortManager::Pin7, PortManager::Output);          // PC7
  IoPin channel = IoPin(PortManager::Port3, PortManager::Pin5, PortManager::Input);        // P35
  AnalogPin cpu_Analog1 = AnalogPin(PortManager::Port4, PortManager::Pin6, PortManager::Input);  // P46
  AnalogPin cpu_Analog2 = AnalogPin(PortManager::Port4, PortManager::Pin2, PortManager::Input);  // P42
  AnalogPin cpu_Analog3 = AnalogPin(PortManager::Port4, PortManager::Pin1, PortManager::Input);  // P41
  AnalogPin cpu_Analog4 = AnalogPin(PortManager::Port4, PortManager::Pin0, PortManager::Input);  // P40
  static IoPin out1 = IoPin(PortManager::PortE, PortManager::Pin0, PortManager::Output);      // PE0
  static IoPin out2 = IoPin(PortManager::PortE, PortManager::Pin7, PortManager::Output);      // PE7
  static IoPin out3 = IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output);      // PA1
  static IoPin out4 = IoPin(PortManager::PortE, PortManager::Pin3, PortManager::Output);      // PE3
  static IoPinGroup moduleOutputs = IoPinGroup(4, &out1, &out2, &out3, &out4);
  Multiplexor ProgramConfig::multiplexor = Multiplexor(mux0, mux1, mux2, channel);
  static MuxIoPin in1 = MuxIoPin(&ProgramConfig::multiplexor, 4);
  static MuxIoPin in2 = MuxIoPin(&ProgramConfig::multiplexor, 2);
  static MuxIoPin in3 = MuxIoPin(&ProgramConfig::multiplexor, 6);
  static MuxIoPin in4 = MuxIoPin(&ProgramConfig::multiplexor, 1);
  // static MuxIoPin in5 = MuxIoPin(&ProgramConfig::multiplexor, 7);
  // static MuxIoPin in6 = MuxIoPin(&ProgramConfig::multiplexor, 5);
  // static MuxIoPin in7 = MuxIoPin(&ProgramConfig::multiplexor, 0);
  // static MuxIoPin in8 = MuxIoPin(&ProgramConfig::multiplexor, 3);
  static IoPinGroup moduleInputs = IoPinGroup(4, &in1, &in2, &in3, &in4);

  IoPin* cpxResetPin = new IoPin(PortManager::Port2, PortManager::Pin6, PortManager::Output);  // P26
  ***/

  // static IoPin plcJoinLED = IoPin(PortManager::PortB, PortManager::Pin3, PortManager::Output);  // PB3
#endif
// Initialization of the serial interfaces to the modem and to the external receiver
#define CONFIG_UART_INIT
#ifdef CONFIG_UART_INIT
/***
IRandGenerator* randGenPtr = new SimpleRandGenerator();
AutostartFlow ProgramConfig::autostartFlow = AutostartFlow(&termDrive, *plcModemPtr, randGenPtr, ledFlow_);
IoCommandManager ProgramConfig::ioCmdManger_ = IoCommandManager(*ProgramConfig::getTimeOutWatcherPtr(),
***/
AutostartFlow *ProgramConfig::autostartFlow = NULL;
IoCommandManager *ProgramConfig::ioCmdManger_ = NULL;

#endif


#endif

// Initialization of the variables of the ProgramConfig class for modem assignment and process management
#define CONFIG_VARIABLE_INIT
#ifdef CONFIG_VARIABLE_INIT
    int ProgramConfig::plcBoardID = BOARD_ID;
    bool ProgramConfig::coordinatorStatus = (BOARD_ID == COORDINATOR_ID);

    WatchDog ProgramConfig::watchDog = WatchDog();
    CommandInterpreter ProgramConfig::terminal = CommandInterpreter(terminalDriverPtr);
    SwitchMatrixAdmin* ProgramConfig::switchMatrixAdminPtr = NULL;
    bool ProgramConfig::isFirstInit50ms = true;
    bool ProgramConfig::blockStoreFlashRouteTable = true;
    bool ProgramConfig::isTestAutomation_ = true;

    ModemInterpreter* ProgramConfig::plcModemPtr = 0;
    TimeMeasurement ProgramConfig::timeMeas = TimeMeasurement();
    TimeMeasurement::measureJob* ProgramConfig::aliveTimeJobPtr = 0;
    TimeMeasurement::measureJob* ProgramConfig::aliveRoundJobPtr = 0;
    TimeMeasurement::measureJob* ProgramConfig::runTimeJobPtr = 0;
#endif

    // Initialization of test
    // ToDo(AME): Check if this is required here
    UnitSuiteTests ProgramConfig::unitSuiteTests = UnitSuiteTests(terminalDriverPtr);
    IntegrationsTests ProgramConfig::integrationsTest = IntegrationsTests(terminalDriverPtr, *aliveForwardingFlow_);


/***
 * constructor of ProgramConfig, which ProgramConfig specific terminal commands
 * mounts it in the terminal interface and reads the FrameCounter from the ROM
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
ProgramConfig::ProgramConfig() {
    pcbInit();

    CodeFlashEditor codeEditor;
    termFilterPtr_ = new TermContextFilter(&termDrive);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    new i2c_app(&termDrive);
    i2c_app::i2cMacRead(const_cast<char *>(""));

    FlashMemoryN25Q_ = new FlashMemoryN25Q(&termDrive);

    SimpleFlashEditor::initFlash();
    int tries = 3;
    while ((SimpleFlashEditor::readFlashData() == false) && (tries--)) {}
    plcBoardID = SimpleFlashEditor::getFlashA().board_ID;
    if (isTestAutomation_) {
        uint16_t read = 0;
        IoPinGroup *inputs = PCBPortSelector::getGroup(PCBPortSelector::ModuleInGroup);
        if (PCBPortSelector::isValidGroup(inputs)) {
            read = inputs->get2ByteForLevel(PortManager::High);
        }
        if ((read & 1) == 0) {  // K1 jumper set => Coordinator in TestBox
            plcBoardID = COORDINATOR_ID;
        } else {
            plcBoardID = 1;
        }
        SimpleFlashEditor::preSetFlashA(COORDINATOR_ID == plcBoardID);
        SimpleFlashEditor::setSysParamID((uint16_t) plcBoardID);
    } else {
        if (0 > tries) {
            plcBoardID = SimpleFlashEditor::getFlashA().board_ID;
        } else {
            plcBoardID = COORDINATOR_ID;
            SimpleFlashEditor::preSetFlashA(COORDINATOR_ID == plcBoardID);
            SimpleFlashEditor::setSysParamID((uint16_t) plcBoardID);
        }
    }

    // Hardware dependent initialization
    IoPin *CPX3Boot = PCBPortSelector::getPin(PCBPortSelector::CPX3Boot);
    if (PCBPortSelector::isValidPin(CPX3Boot)) {
#if CPX_BOOT0_MODE == 1
        CPX3Boot->setLevel(PortManager::High);
#elif HW_VERSION >= 1
        CPX3Boot->setLevel(PortManager::Low);
#endif
    }

    IoPin *FCC_CENELC_A1 = PCBPortSelector::getPin(PCBPortSelector::FCC_Cenelec_Out, 1);
    IoPin *FCC_CENELC_A2 = PCBPortSelector::getPin(PCBPortSelector::FCC_Cenelec_Out, 2);
    if (PCBPortSelector::isValidPin(FCC_CENELC_A1) && PCBPortSelector::isValidPin(FCC_CENELC_A2)) {
        FCC_CENELC_A1->setLevel(PortManager::High);  // Low = FCC
        FCC_CENELC_A2->setLevel(PortManager::High);  // Low = FCC
    }
    IoPin *Backup = PCBPortSelector::getPin(PCBPortSelector::CPXBackup);
    if (PCBPortSelector::isValidPin(Backup)) {
        Backup->setLevel(PortManager::High);
    }

    ModemController::setMacShortAddress((uint16_t) plcBoardID);
    // ModemController::setTermPtr(terminalDriverPtr);
    setCoordinatorStatus(plcBoardID == COORDINATOR_ID);
    IDiscoveryManager *simplDiscMangerPtr = new SimpleDiscoveryManager(
            terminalDriverPtr);
    IoPin *cpxResetPin = PCBPortSelector::getPin(PCBPortSelector::CPXResetPin);
    if (PCBPortSelector::isValidPin(cpxResetPin)) {
        // cpxResetPin->setLowActiv(IoPin::aktiv);
        plcModemPtr = new ModemInterpreter(modemDrive, termDrive, cpxResetPin,
                (uint8_t) getPlcBoardId(), simplDiscMangerPtr,
                simpleTimeOutWatcherPtr, *ioCmdManger_, *assignmentFlow_, *ledFlow_,
                *aliveJumpMapper_);
    }
    IoPinGroup* inputs = PCBPortSelector::getGroup(PCBPortSelector::ModuleInGroup);
    IoPinGroup* outputs = PCBPortSelector::getGroup(PCBPortSelector::ModuleOutGroup);
    if (PCBPortSelector::isValidGroup(inputs) && PCBPortSelector::isValidGroup(outputs)) {
        switchMatrixAdminPtr = new SwitchMatrixAdmin(outputs, inputs,
                (uint16_t) plcBoardID, *ioCmdManger_);
    }
//  plcModemPtr->setFrameCounter(simpleFlashEditor::getFrameCounter()+deadTimeOffset);
//
//
    ConfigTermConnector termConnector = ConfigTermConnector(
            simpleTimeOutWatcherPtr, termFilterPtr_, routingManagerPtr,
            &unitSuiteTests, autostartFlow, &modemDrive);
    static termCommand* updateTab = CommandTabProvider::getCmdTab(CommandTabProvider::update);
    static termCommand* configTab = CommandTabProvider::getCmdTab(CommandTabProvider::config);

    updateTab = UpdateTermConnector::fillConnectionsInTab(updateTab);
    configTab = termConnector.fillConnectionsInTab(configTab);

    CommandInterpreter::cmdListTab.add((void *)configTab);
    CommandInterpreter::cmdListTab.add((void *)updateTab);
    PlcTerminalCommandConnector::addCmdTab2List();
    cpxProfilePtr->setModemPtr(plcModemPtr);
    // plcProfilePtr->setModemPtr(plcModemPtr);
    static DelayHandler delyHandler;
    static IEventStateProvider* stateControll = new SystemStateController();
    _deployFlow = new SoftwareDeploymentFlow(reinterpret_cast<ITransmitter*>(plcModemPtr->getModemControllPtr()),
            reinterpret_cast<uint8_t*>(FLASH_CF_BLOCK_400), delyHandler, IRoutingManager::BCGlobalIP, stateControll);
}

/***
 * Not currently needed
 */
ProgramConfig::~ProgramConfig() {
}

void ProgramConfig::pcbInit() {
    pcbPortSelector_ = new PCBPortSelector();
#if HW_VERSION == 12
    pcbPortSelector_->init(PCBPortSelector::Version1_2);
#elif HW_VERSION == 1
    pcbPortSelector_->init(PCBPortSelector::Version1_1);
#else
    pcbPortSelector_->init(PCBPortSelector::Evaluation);
#endif
    IoPin *led1 = pcbPortSelector_->getPin(PCBPortSelector::LED, 1);
    IoPin *led2 = pcbPortSelector_->getPin(PCBPortSelector::LED, 2);

    ledFlow_ = new LEDSignalFlow(led1, led2);

    // BGroupForwarding* ProgramConfig::
    groupForwardingPtr = new BGroupForwarding(&termDrive, *(routingManager), *simpleTimeOutWatcherPtr, *ledFlow_);

    IRandGenerator* randGenPtr = new SimpleRandGenerator();
    autostartFlow = new AutostartFlow(&termDrive, *plcModemPtr, randGenPtr, *ledFlow_);
    IoPinGroup* inputs = PCBPortSelector::getGroup(PCBPortSelector::ModuleInGroup);
    IoPinGroup* outputs = PCBPortSelector::getGroup(PCBPortSelector::ModuleOutGroup);
    if (PCBPortSelector::isValidGroup(inputs) && PCBPortSelector::isValidGroup(outputs)) {
        ioCmdManger_ = new IoCommandManager(*ProgramConfig::getTimeOutWatcherPtr(),
                            &termDrive, *ledFlow_, *inputs, *outputs);
    }

    assignmentFlow_ = new AssignmentFlow(termDrive, *ledFlow_);
    aliveForwardingFlow_ = new ForwardingFlow(groupForwardingPtr, *ioCmdManger_,
                            *simpleTimeOutWatcherPtr, &termDrive);
    aliveJumpMapper_ = new JumpMapper(simpleTimeOutWatcherPtr, &termDrive, *assignmentFlow_);
    plcProfile_ = new SerialPlcProfile(*aliveForwardingFlow_);
}

// TODO(AME): Check
void ProgramConfig::window100micro() {
#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
#endif
}

/***
 * 100µs routine, which incoming PLC requests during
 * the PLC auto start receives and interpret
 */
void ProgramConfig::bootWindow100micro() {
#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
    if (!isFirstInit50ms) {
        plcModemPtr->handelBootloading();
    }
#endif
}

/***
 * 1 ms routine, which incoming PLC requests after
 * the PLC auto start receives and interpret
 * Furthermore, in addition “feeding” the watch dog and
 * the alive query is also carried out
*/
void ProgramConfig::window1ms() {
#if IS_FIRMWARE_DOWNLOAD_ENABLED >= 1
    // if (plcModem.isNotUploading()) {
#else
    {
#endif
    if (autostartFlow->getPlcAutoStart() > AutostartFlow::initModemStep) {
        plcModemPtr->handelRequests();
    }
    WatchDog::feed();
#if ALIVE_CHECK_LEVEL == 1
    static uint16_t checkTimeCount = 0;
    if (coordinatorStatus && ((checkTimeCount++ >ModemController::getAliveCheckInterval()) ||
                            ModemController::isCheckMemberPending())) {  // && !plcModem.isNewMemberRequest()
        ModemController::checkNetworkMembers();
        checkTimeCount = 0;
    }
#endif
}

void ProgramConfig::window5ms() {
    if (autostartFlow->getPlcAutoStart() > AutostartFlow::initModemStep && !autostartFlow->isFinished()) {
        plcModemPtr->handelRequests();
    }
}


/***
 * 50ms routine, which manages the incoming and outgoing terminal commands.
 * In addition, interpreted PLC messages processed step by step and the tasks for them
 * SwitchMatrix is processed.
*/
void ProgramConfig::window50ms() {
    SerialComInterface::getInstance(sci12)->popTXD();
    SerialComInterface::getInstance(sci5)->pushRXD();
    ledFlow_->processSignals();
    simpleTimeOutWatcherPtr->update();
    if (autostartFlow->isFinished()) {
        terminal.TerminalDump();
        plcModemPtr->answerStepwiseOnIncommingMessages();
        aliveForwardingFlow_->updateRoundTrip();
        aliveForwardingFlow_->handleTransmissionFailure();
        // switchMatrixAdminPtr->sendOutstandingPLCCommandsAndExecuteSignalCalls(plcModemPtr);
    }
    terminal.ExecuteLocalCmd();
}


void ProgramConfig::bootWindow50ms() {
    WatchDog::feed();
    ledFlow_->processSignals();
    simpleTimeOutWatcherPtr->update();
    if (isFirstInit50ms) {
        isFirstInit50ms = false;
    } else {
        autostartFlow->boot();
        SerialComInterface::getInstance(sci12)->popTXD();
    }
}


/***
 * 1s routine, which the PLC auto start and the LED control
 * manages. Furthermore, incoming signals from the switch matrix are
 * interpreted and, if necessary, repeated outgoing PLC signals
*/
void ProgramConfig::window1s() {
    autostartFlow->proceed();

    if (isFirstInit50ms) {
        isFirstInit50ms = false;
    } else {
        if (!ProgramConfig::isCoordinator() && autostartFlow->isFinished()) {
            // switchMatrixAdminPtr->pollSignalCommandsWithModem();
            // switchMatrixAdminPtr->repeatPLCCommandWithResponseTimeout(plcModemPtr);
        }

        // pushRXD12(); // triggers at reset after 13 minutes
        SerialComInterface::getInstance(sci5)->pushRXD();
        _deployFlow->proceed();
        // TODO(AME): create a function for this section and move to the Alive Flow
//      if(ModemController::isAliveCheckEnabled()) {
//          uint16_t act = ModemController::getAliveCheckActLoops();
//          uint16_t preset= ModemController::getAliveCheckPresetLoops();
//          if (act == preset) {
//              uint16_t next = ModemController::getAliveNextGroupe();
//              if (next) {
//                  ModemController::triggerSendBroadCastDataWithLengthToGroupeIP((uint8_t*)"::A",3, next);
//                  if (preset) {
//                      ModemController::setAliveCheckActLoops(--act);
//                  }
//              }
//          }
//      }
    }
}

void ProgramConfig::window60s() {
    if (ModemController::isInitFinish()) {
        if (!blockStoreFlashRouteTable) {
            bool lGet = false;
            if (ProgramConfig::isCoordinator()) {
                ModemController::disableNetMemberAfterPendingTimeout();
                if (ModemController::getNetMemberPtrByIndex(1)->getNetworkIPAddress()
                        != 0xFF) {
                    lGet = true;
                }
            } else {    // peer
                if (ModemController::isConnectedToCoordinator() && autostartFlow->isFinished()) {
                    lGet = true;
                }
            }
            if (lGet) {
                ModemController::setProcessState(
                        ModemController::createGetForceJoinInfo(true).getHead());
            }
        }
        aliveForwardingFlow_->window60s(ProgramConfig::isCoordinator());
    }
}


/***
 * This Method set the Serial Profile of the Modem Serial Driver to plcProfile
 * This is necessary to interpret the incoming Serial Frames in G3-Specific way
 */
void ProgramConfig::setPLCProfileForModemDriver() {
    modemDrive.getSciPtr()->setProfilePtr(plcProfile_);
}


}  // namespace AME_SRC

