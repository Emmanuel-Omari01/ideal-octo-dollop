/*
 * AutostartFlow.cpp
 *
 *  Created on: 14.02.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "AutostartFlow.h"

#include "../HelpStructures/CharOperations.h"
#include "../PlcCommunication/PlcTerminalCommandConnector.h"
#include "../StorageManagement/SimpleFlashEditor.h"
#include "../SystemBase/SystemInformation.h"
#include "../SystemLogic/StorageAdministrator.h"
#include "../SystemBase/MicrocontrollerConfig.h"

namespace AME_SRC {

#define debug_level_local 1   // values >0 enables local debug terminal outputs

// State variables for boot loop (load firmware to CPX) and auto start loop (init CPX and start/join Network)
AutostartFlow::autostartStep AutostartFlow::bootAutoStart = initMsg;
AutostartFlow::autostartStep AutostartFlow::plcAutoStart = initModemStep;
#ifdef USE_preset_Search_Mode
bool AutostartFlow::presetSearchMode = false;
#endif
bool    AutostartFlow::proceed_again = false;
// Force Join check at coordinator
uint8_t AutostartFlow::CheckNextPeerIdx = 1;
uint8_t AutostartFlow::CheckCallOuts = 1;       // coordinator
int16_t AutostartFlow::CheckDelay = 0;

// Force Join check at peer
uint16_t AutostartFlow::FJPeerActTime = 0;   // actual time [s].
                                             // => FJPeerActTime>ReloadWaitP2CVerification::timeout? yes: Timeout.
uint8_t  AutostartFlow::FJPeerCounterJ = 0;  // Counter to create a delay for sending the next verification request
uint8_t  AutostartFlow::plcFlowState = plcInit;
uint8_t  AutostartFlow::ignoreNTimesFJRequests = 0;
TermContextFilter *AutostartFlow::termFilterPtr_ = NULL;


/***
 * Constructor of Class AutostartFlow
 * @param newSerialDriver	=> This Driver Class is used to print Status Messages on the Terminal
 * @param newModemInterpreterReff => Interpreter Reff for setting Status in the Interpreter (TODO: Maybe Obsolete should be replaced )
 */
AutostartFlow::AutostartFlow(SerialDriver* newSerialDriver, ModemInterpreter& newModemInterpreterReff,
                IRandGenerator* rG, LEDSignalFlow& ledFlow):
                modemInterpreterReff(newModemInterpreterReff),
                bestFoundDescriptorPtr(NULL), autostartFinishedStatus(false), randomGen(rG), ledFlowReff_{ledFlow} {
    termFilterPtr_ = new TermContextFilter(newSerialDriver);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
}

AutostartFlow::~AutostartFlow() = default;

/**
 * Debug function to skip peer auto-search loop and proceed to main Menu
 * @param dummy => just used for function signature restrictions
 * @return German Message for Terminal Usage
 */
char* AutostartFlow::exitAutoStart(__attribute__((unused)) char* dummy) {
    plcAutoStart = announceFinishStep;
    return (const_cast<char *>(TX::getText(TX::cTxtAutoStartEnd)));
}

/***
 * This method controls the auto boot step chain of the plc-modem
 * It resets the modem and parameterize the bootloader-class so
 * that he can load the plc firmware
 */
void AutostartFlow::boot() {
    switch (bootAutoStart) {
    case initMsg:
        termFilterPtr_->println(TX::BootSystemStart);
        termFilterPtr_->println(SystemInformation::resetSourceToString(Global2::OutBuff,
                        Global2::outBuffMaxLength));
        bootAutoStart = resetModemStep;
        break;
    case resetModemStep:
        resetModem();
        break;
    case waitBootloaderStep:
        waitBootloaderIfNecessary();
        break;
    default:
        break;
    case deactivated:
        break;
    case waitSystemRestart:
        break;
    }
}

/** This method proceeds the plc auto start logic
 * The step chain consists of modem initialization;
 * networkSearch and networkJoin
 */
void AutostartFlow::proceed() {
  restart_proceed:
    proceed_again = false;

    // if (plcAutoStart>=waitJoinConfirmStep) {
    //      sprintf(Global2::OutBuff,"AutoStartFlow step:%d", plcAutoStart);
    //      termFilterPtr_->println(Global2::OutBuff);
    // }

    switch (plcAutoStart) {
    case initModemStep:
        ModemController::finishInitNetworkMember();
        initModem();
        break;
    case waitInitConfirmStep:
        waitInitConfirm(autostartStep::checkForceJoinTabStep);
        break;
    case checkForceJoinTabStep: {
        if (termFilterPtr_->isNotFiltered(TermContextFilter::filterInitLoadBar)
                && !ProgramConfig::isCoordinator()) {
            termFilterPtr_->println("#|");
            termFilterPtr_->print(TX::getText(TX::InitializationSuccessfulMsg));
            termFilterPtr_->println("");
            termFilterPtr_->print(TX::getText(TX::StartNetworkSearchMsg));
            termFilterPtr_->println("\r|---------------|");
        }

        autostartStep deviceDefaultStep =
                (ProgramConfig::isCoordinator()) ?
                        announceFinishStep : searchNetworkStep;
#ifdef USE_preset_Search_Mode
        if (presetSearchMode) {  // true: Force Join Failed
            plcAutoStart = deviceDefaultStep;
            set_proceed_again();
            break;
        }
#endif
        if (checkForceJoinTabs()) {
            plcAutoStart = (SimpleFlashEditor::isForceJoinMode()) ? startupReloadStep : deviceDefaultStep;
        } else {
            plcAutoStart = deviceDefaultStep;
        }
    }
        break;

        // ForceJoin
    case startupReloadStep:
    case waitReloadStep:
    case ReloadJoinStep:
    case ReloadJoinConfirmStep:
    case PeerFJProofJoinStep:
    case ReloadWaitRandomStep:
    case ReloadSendP2CVerification1a:
    case ReloadSendP2CVerification1b:
    case ReloadSendP2CVerification2a:
    case ReloadSendP2CVerification2b:
    case ReloadSendP2CVerification3a:
    case ReloadSendP2CVerification3b:
        // Coordinator
    case ReloadCoordVerifyStart:
    case ReloadCoordVerifyLoopStart:
    case ReloadCoordVerifySendPeer:
    case ReloadCoordInitDelay:
    case ReloadCoordWaitPeerDelay:
    case ReloadCoordVerifyLoopNext:
    case ReloadWaitTimeout:
        startupReloadAndJoin();
        break;

        // startupSearchAndJoin (start)--------------
    case searchNetworkStep:
    case waitTillNextSearchStep:
    case proofSearchResultsStep:
    case joinNetworkStep:
    case waitJoinConfirmStep:
    case proofConnectionStep:
    case connectionProofedStep:
    case waitAfterJoinFailStep:
    case storeJoinInfoStep:
        startupSearchAndJoin();
        break;
        // startupSearchAndJoin (end)--------------
    case announceFinishStep:
        announceFinish();
        break;
    case deleteNetworkAgentStep:
        deleteNetworkAgent();
        break;
    case runTests: {
    	UnitSuiteTests *pts = ProgramConfig::getUnitSuiteTestsPtr();
        IntegrationsTests *pIntTest = ProgramConfig::getIntegrationTestPtr();
        int nTested = 0;
        if (pts->isTested()) {
            nTested += 1;
        } else {
            pts->runTests();
        }
        if (pIntTest->isTested()) {
            nTested += 1;
        } else {
            pIntTest->runTests();
        }
        if (nTested == 2) {
            plcAutoStart = deactivated;
        }
    }
        break;
    case deactivated:
        break;
    case waitSystemRestart:
        break;
    default:
        break;
    }
    if (proceed_again) {
        goto restart_proceed;
    }
}

// Execute subset of steps for loading the routing tables and the forced join process
void AutostartFlow::startupReloadAndJoin() {
#if debug_level_local > 0
    static uint8_t last_state = 0xFF;
    static uint8_t last_step = 0xFF;
#endif
    const uint8_t aliveTimeOutDelay = 30;
    ModemInterpreter::sForceJoinState state =
            modemInterpreterReff.getForceJoinState();
    if (state & ModemInterpreter::FJpending) {
        ++FJPeerActTime;
    }
#if debug_level_local > 0
    if (termFilterPtr_->isNotFiltered(TermContextFilter::filterForceJoinS)) {
        if (last_step != plcAutoStart) {
            last_step = plcAutoStart;
            termFilterPtr_->println("");
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowForceJoinStep), plcAutoStart);
            termFilterPtr_->print(Global2::OutBuff);
        }
        if (last_state != state) {
            last_state = state;
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowForceJoinState), state);
            termFilterPtr_->print(Global2::OutBuff);
        }
    }
#endif
    switch (plcAutoStart) {
    case startupReloadStep:
        ModemController::setRequestRepeatMode(false);
        ModemController::setProcessState(
                ModemController::createSetForceJoinInfo(ModemController::start).getHead());
        termFilterPtr_->println(TX::cTxtExitSearchMsg);
        plcAutoStart = waitReloadStep;
        break;
    case waitReloadStep:
        if (ModemInterpreter::getModemControllPtr()->getControllerState()
                == ModemController::idle) {
            plcAutoStart = ReloadJoinStep;
            set_proceed_again();
        } else {
            getTimeOutStatusAndSetFlowStep(2, ReloadJoinStep);
        }
        break;
    case ReloadJoinStep:
        FJPeerActTime = 0;
        forceJoinNetwork(ReloadJoinConfirmStep);  // => plcAutoStart:=
                                                  // coordinator: ReloadCoordVerifyStart,
                                                  // Peer: ReloadJoinConfirmStep
        break;
        // Peer Force-Join
    case ReloadJoinConfirmStep:
        if (!waitJoinConfirm(PeerFJProofJoinStep, 2)) {
            break;
        }
        set_proceed_again();
        break;
    case PeerFJProofJoinStep:
        profJoinConnection(ReloadWaitRandomStep, ReloadJoinStep,
                searchNetworkStep);
        break;
    case ReloadWaitRandomStep: {
        uint16_t mode = SimpleFlashEditor::getAutoStartFlowMode();
        if (mode & SimpleFlashEditor::ForceJoinWithVerification) {
            waitRandomTillJoin(randomGen);
        } else if (mode & SimpleFlashEditor::ForceJoinWithTimeOut) {
            ITimeOutWatcher::timeOutInfo *timeOutPtr;
            timeOutPtr = ITimeOutWatcher::createDefaultTimeOut(
                ProgramConfig::forceJoinAliveTOutID,
                resetSoftwareForDefaultSearch, 0);

            timeOutPtr->delay = aliveTimeOutDelay * 20;
            timeOutPtr->timeCounter = timeOutPtr->delay;
            timeOutPtr->timeCounter = 1;
            timeOutPtr->timeOutAmount = 1;
            ProgramConfig::getTimeOutWatcherPtr()->registerTimeout(*timeOutPtr);
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::WaitForAliveMsg), aliveTimeOutDelay);
            termFilterPtr_->println(Global2::OutBuff);
            termFilterPtr_->println("|------------------------------|");
            termFilterPtr_->print("|");
            ledFlowReff_.setLedMode(LEDSignalFlow::forceJoin);
            plcAutoStart = ReloadWaitTimeout;
        } else {
            plcAutoStart = announceFinishStep;
            set_proceed_again();
        }
        break;
    }
    case ReloadSendP2CVerification1a:
        ReloadSendP2CVerificationMsg();
        // FJPeerTimeout = simpleFlashEditor::getFJDelay(simpleFlashEditor::fjDelayX);
        FJPeerActTime = 0;
        plcAutoStart = ReloadSendP2CVerification1b;
        break;
    case ReloadSendP2CVerification1b:
        ReloadWaitP2CVerification();
        getTimeOutStatusAndSetFlowStep(5, ReloadSendP2CVerification2a);
        break;
    case ReloadSendP2CVerification2a:
        ReloadSendP2CVerificationMsg();
        FJPeerCounterJ = 2;
        plcAutoStart = ReloadSendP2CVerification3b;
        break;
    case ReloadSendP2CVerification3a:
        FJPeerCounterJ++;
#if debug_level_local > 0
        if (termFilterPtr_->isNotFiltered(TermContextFilter::filterForceJoinS)) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowVerificationRequests),
                    FJPeerCounterJ);
            termFilterPtr_->println(Global2::OutBuff);
        }
#endif
        ReloadSendP2CVerificationMsg();
        getTimeOutStatusAndSetFlowStep(0, ReloadSendP2CVerification2b);  // Reset waitCount
        plcAutoStart = ReloadSendP2CVerification3b;
        break;
    case ReloadSendP2CVerification2b:
    case ReloadSendP2CVerification3b:
        if (ReloadWaitP2CVerification()) {
            uint16_t waitTime = (uint16_t) (FJPeerCounterJ
                                * SimpleFlashEditor::getFJDelay(SimpleFlashEditor::fjDelayZ));
            getTimeOutStatusAndSetFlowStep(waitTime, ReloadSendP2CVerification3a);
        }
        break;
    case ReloadWaitTimeout: {
        static uint8_t timeCount = 1;
        termFilterPtr_->putchar('#');
        if (++timeCount > aliveTimeOutDelay) {
            termFilterPtr_->println("|");
        }
        if (IAliveForwardBehaviour::isMessageReceived()) {
            ProgramConfig::getTimeOutWatcherPtr()->stopAndReset(ProgramConfig::forceJoinAliveTOutID);
            plcAutoStart = announceFinishStep;
            set_proceed_again();
        }
        break;
    }
        // ---------------- Coordinator force join check ---------------------------
    case ReloadCoordVerifyStart:
    case ReloadCoordVerifyLoopStart:
    case ReloadCoordVerifySendPeer:
    case ReloadCoordInitDelay:
    case ReloadCoordWaitPeerDelay:
    case ReloadCoordVerifyLoopNext:
        checkForceJoinCoordinator();
        break;
    default:
        break;
    }
}

// sub steps to check if the force join to the coordinator was successful
void AutostartFlow::checkForceJoinCoordinator() {
    NetworkMember *pn;
    uint32_t faktor, scale;
    switch (plcAutoStart) {
    case ReloadCoordVerifyStart:
        CheckNextPeerIdx = 1;
        CheckCallOuts = 1;
        if (ReloadCoordCheckPeers()) {
            plcAutoStart = announceFinishStep;
            break;
        }
        plcAutoStart = ReloadCoordVerifyLoopStart;
        set_proceed_again();
        break;
    case ReloadCoordVerifyLoopStart:
        // check, if root discovery was successful
        pn = ModemController::getNetMemberPtrByIndex((uint16_t) CheckNextPeerIdx);
        if (pn == NULL) {
            plcAutoStart = ReloadCoordVerifyLoopNext;
            set_proceed_again();
            break;
        }
        if (!pn->connectedOrNotVerified() && (pn->getNetworkIPAddress() != 0xFF)) {  // Is the peer available?
            plcAutoStart = ReloadCoordVerifyLoopNext;
        } else {
            plcAutoStart = ReloadCoordVerifySendPeer;
        }
        set_proceed_again();
        break;
    case ReloadCoordVerifySendPeer:
        if (ReloadSendC2PVerificationMsg(CheckNextPeerIdx)) {  // send
            plcAutoStart = ReloadCoordInitDelay;
        } else {
            plcAutoStart = ReloadCoordVerifyLoopNext;
        }
        set_proceed_again();
        break;
    case ReloadCoordInitDelay:
        // Determine dwell
        faktor = SimpleFlashEditor::getFJDelay(SimpleFlashEditor::fjDelayX);
        if ((faktor == 0) || (faktor > 180))
            faktor = 2;
        scale = SimpleFlashEditor::getFJScale();
        if ((scale == 0) || (scale > 20))
            scale = 2;
        CheckDelay = (int16_t) (scale * CheckCallOuts * faktor);
        plcAutoStart = ReloadCoordWaitPeerDelay;
        break;
    case ReloadCoordWaitPeerDelay:
        termFilterPtr_->putchar('$');
        if (--CheckDelay <= 0) {
            plcAutoStart = ReloadCoordVerifyLoopNext;
        }
        break;

        // select next peer if he is available. In other cases restart loop due to callout value
    case ReloadCoordVerifyLoopNext:
        if (++CheckNextPeerIdx <= ModemController::getNetworkMemberMax()) {
            plcAutoStart = ReloadCoordVerifyLoopStart;
            set_proceed_again();
            break;
        }
        if (ReloadCoordCheckPeers()) {
            plcAutoStart = announceFinishStep;
            break;
        }

        // are more loops available?
        if (++CheckCallOuts < SimpleFlashEditor::getFJCallout()) {
            CheckNextPeerIdx = 1;
            plcAutoStart = ReloadCoordVerifyLoopStart;
            break;
        }
        ReloadCoordDelPeers();
        plcAutoStart = announceFinishStep;
        break;
    default:
        break;
    }
}

// Execute subset of steps which are used to search in network within the join process
void AutostartFlow::startupSearchAndJoin() {
    switch (plcAutoStart) {
        case searchNetworkStep:
            searchNetwork();
            break;
        case waitTillNextSearchStep:
            waitTillNextSearch();
            break;
        case proofSearchResultsStep:
            proofSearchResults();
            break;
        case joinNetworkStep:
            joinNetwork();
            plcAutoStart = waitJoinConfirmStep;
            break;
        case waitJoinConfirmStep:
            waitJoinConfirm(proofConnectionStep, 2);    // ,15 [s]
            break;
        case proofConnectionStep:
            profJoinConnection(storeJoinInfoStep, waitAfterJoinFailStep, deleteNetworkAgentStep);
            break;
        case storeJoinInfoStep:
            termFilterPtr_->println(TX::StoreJoinInfoMsg);
            ModemController::setProcessState(
                ModemController::createGetForceJoinInfo(true).getHead());
            plcAutoStart = connectionProofedStep;
            break;
        case connectionProofedStep:
            doConnectedAction();
            set_proceed_again();
            plcAutoStart = announceFinishStep;
            break;
        case waitAfterJoinFailStep:
            waitAfterJoinFail();
            break;
        default:
            break;
    }
}

/** Method to reset the Modem
 * It clears sci5 Uart Errors, ensures Watchdog
 * reset is not set and trigger a reset signal
 * on the reset pin controlled by the modem
 */
void AutostartFlow::resetModem() {
    SerialComInterface::getInstance(sci5)->pushRXD();
    WatchDog::setRestartFlag(false);
    ModemController::resetModem();
    bootAutoStart = waitBootloaderStep;
    return;
}

#define AUTOSTART_FLOW_INITMODEM
#ifdef AUTOSTART_FLOW_INITMODEM
void AutostartFlow::initModem() {
    setModemProfileAndStartTimeMeasurement();
    initPLCModem();
}

void AutostartFlow::setModemProfileAndStartTimeMeasurement() {
    TimeMeasurement::restart();
    ProgramConfig::setRunTimeJobPtr(&TimeMeasurement::registerTimedJob(-1));
    ProgramConfig::setPLCProfileForModemDriver();
}

/***
 * The Methods use the Terminal-Init Method to init the Modem
 * Whether the module is coordinator or peer a specific message is
 * which creates a plc-command step chain controlled by the ModemController
 */
void AutostartFlow::initPLCModem() {
    if (ProgramConfig::isCoordinator()) {
        termFilterPtr_->println(
            PlcTerminalCommandConnector::callInitChannelWithMode(const_cast<char*>(TX::getText(TX::cTxtPlcInitC))));
    } else {
        termFilterPtr_->println(
            PlcTerminalCommandConnector::callInitChannelWithMode(const_cast<char*>(TX::getText(TX::cTxtPlcInitP))));
    }
    plcAutoStart = waitInitConfirmStep;
    setPlcFlowState(tPlcFlowState::plcInit);
}
#endif


#define AUTOSTART_FLOW_WAIT_BOOTLOADER
#ifdef AUTOSTART_FLOW_WAIT_BOOTLOADER
void AutostartFlow::waitBootloaderIfNecessary() {
    if (ModemController::isFirmenwareLoaded()) {
        bootAutoStart = deactivated;
    } else {
        evaluateBootloaderTimeOuts();
    }
}

/**
 * Reset the system or the bootloader if a specific time elapse
 */
void AutostartFlow::evaluateBootloaderTimeOuts() {
    evaluateBootTimeOut();
    evaluateResetTimeOut();
}

void AutostartFlow::evaluateBootTimeOut() {
    static uint8_t bootCount = 0;
    uint16_t modemBootTimeOut = 200;    // 200*0.05s = 10s
    if (bootCount++ > modemBootTimeOut) {  // *segNumber
        plcAutoStart = resetModemStep;
        termFilterPtr_->println(TX::cTxtRestartBootLoader);
        bootCount = 0;
    }
}

void AutostartFlow::evaluateResetTimeOut() {
    uint16_t modemResetTimeOut = 1000;
    bool isModemResetTimeout = getTimeOutStatusAndSetFlowStep(modemResetTimeOut, waitSystemRestart);
    if (isModemResetTimeout) {
        termFilterPtr_->println(TX::cTxtRestartSystem);
        WatchDog::setRestartFlag(true);
    }
}
#endif


#define AUTOSTART_FLOW_WAIT_INIT
#ifdef AUTOSTART_FLOW_WAIT_INIT
void AutostartFlow::waitInitConfirm(autostartStep startupStep) {
    if (ModemController::isInitFinish()) {
        plcAutoStart = startupStep;  // ProgramConfig::isCoordinator() ? announceFinishStep : startupStep;
        set_proceed_again();
    } else {
        evaluateInitResetTimeOut();
    }
}

void AutostartFlow::evaluateInitResetTimeOut() {
    uint16_t modemResetTimeOut = 500;
    bool isInitTimeout = getTimeOutStatusAndSetFlowStep(modemResetTimeOut, initModemStep);
    if (isInitTimeout) {
        termFilterPtr_->println(TX::cTxtRestartInit);
    }
}
#endif

/**
 * This Step is needed for re-joining purpose later on
 * If a peer try to re-join a network with a lower frame
 * counter that is tracked by the coordinator it can not
 * join.
 *
 * this method loads a given frame-count into the modem
 * via user terminal commands
 */
 /***
void AutostartFlow::loadFrameCounter() {
    sprintf(Global2::OutBuff,"setMacAttribute 119 %lu",modemInterpreterReff.getFrameCounter());
    // termFilterPtr_->println(PlcTerminalCommandConnector::setMacAttribute(Global2::OutBuff));
    plcAutoStart = searchNetworkStep;
    //plcAutoStart = announceFinish;
    termFilterPtr_->println("");
    termFilterPtr_->println("Der Autostart kann durch \"exitSearch\" abgebrochen werden!");
}
***/

/**
 * This Step trigger a plc-Network search
 * the request and the results are handled by
 * the modemController
 */
void AutostartFlow::searchNetwork() {
    uint8_t searchTimeCount = 15;
    ModemController::setRequestRepeatMode(false);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::searchNetwork, 2, channel0,
            searchTimeCount);
    plcAutoStart = proofSearchResultsStep;
    setPlcFlowState(tPlcFlowState::searching);
    ledFlowReff_.setLedMode(LEDSignalFlow::searchNet);
}

/**
 * defines a delay between search requests
 */
void AutostartFlow::waitTillNextSearch() {
    uint8_t searchWaitTime = 2;
    termFilterPtr_->putchar('#');
    getTimeOutStatusAndSetFlowStep(searchWaitTime, searchNetworkStep);
}

/**
 * defines a random delay before joining the Network
 * This is needed in order to balance the message
 * traffic on coordinator side
 */
void AutostartFlow::waitRandomTillJoin(IRandGenerator* randGen) {
    static uint8_t randWaitTime = 0;
    if (randWaitTime == 0) {
        randGen->setSeed(ModemController::getMacShortAddress());
        randGen->setRange(1, 5);
        randWaitTime = (uint8_t) randGen->generate();
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::ShowRandomDelay), randWaitTime);
        termFilterPtr_->println(Global2::OutBuff);
    } else {
        termFilterPtr_->putchar('?');
    }

    getTimeOutStatusAndSetFlowStep(randWaitTime, ReloadSendP2CVerification1a);
}


/**
 * This Step choose the best Network Agent for the join attempt
 * If it gets the best Network Agent the step chain is proceed to
 * join a network with the chosen network agent as mediator
 */
void AutostartFlow::proofSearchResults() {
    bestFoundDescriptorPtr = ModemController::getbestPanDescriptorFromDescriptorContainer();
    if (bestFoundDescriptorPtr != NULL) {
        plcAutoStart = (bestFoundDescriptorPtr->panId == 0xFFFF) ? waitTillNextSearchStep : joinNetworkStep;
    }
    termFilterPtr_->putchar('#');
    evaluateSearchTimeout();
}

void AutostartFlow::evaluateSearchTimeout() {
    uint8_t searchWaitTimeout = 20;
    bool isSearchTimeout = getTimeOutStatusAndSetFlowStep(searchWaitTimeout, waitTillNextSearchStep);
    if (isSearchTimeout) {
        int job = TimeMeasurement::getJobIndex(*ProgramConfig::getRunTimeJobPtr());
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtTempTime),
                TimeMeasurement::toString((uint8_t) job, tmp, sizeof(tmp), TimeMeasurement::TimeHHMMSSms));
        termFilterPtr_->println(Global2::OutBuff);
        termFilterPtr_->println(TX::cTxtNetScanTimeout);
        modemInterpreterReff.setIsNetworkSearchTriggerd(false);
    }
}

/***
 * Trigger a join attempt for the chosen network agent
 * Inform the user about the join process
 */
void AutostartFlow::joinNetwork() {
    uint16_t panId, address;
    if (bestFoundDescriptorPtr) {
        panId = bestFoundDescriptorPtr->panId;
        address = bestFoundDescriptorPtr->adress;
    } else {
        SimpleFlashEditor::Parameter_A &sys_param = SimpleFlashEditor::getFlashA();
        panId = sys_param.targetNetId;
        address = sys_param.ownNetworkAddress;
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtStartJoiningPan), panId, address);
    termFilterPtr_->println(Global2::OutBuff);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::joinNetwork, 3, channel0, panId, address);
    ledFlowReff_.setLedMode(LEDSignalFlow::joinNet);
}

// Actions after a successful join process
void AutostartFlow::doConnectedAction() {
    if (bestFoundDescriptorPtr) {
        // // store panID and agent address in the data flash memory
        // simpleFlashEditor::Parameter_A &sys_param = simpleFlashEditor::getFlashA();
        // sys_param.targetNetId       = bestFoundDescriptorPtr->panId;
        // sys_param.ownNetworkAddress = bestFoundDescriptorPtr->adress;
        // simpleFlashEditor::updateSysParamFlash();
        SimpleFlashEditor::setTargetNetId(bestFoundDescriptorPtr->panId);

        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%c", ModemInterpreter::commandPrefix,
                ModemInterpreter::JoinActionRequest);
        ModemController::triggerSendDataWithMessageToNetIPAndRoutingMode(Global2::OutBuff, 0, true);

        // activate force join
        uint16_t mode = SimpleFlashEditor::ForceJoinMode | SimpleFlashEditor::ForceJoinWithTimeOut;
        SimpleFlashEditor::setAutoStartFlowMode(mode);
    }
#ifdef USE_preset_Search_Mode
    if (presetSearchMode) {  // Frame-Counter nach Join-Prozess setzen.
                             // TODO(AME): check if the frame counter has always to be set
        ModemController::setProcessState(ModemController::createSetForceJoinInfo(ModemController::fc_only).getHead());
    }
#endif
}

/***
 * Wait delay in order to give the modem time to process the join attempt
 */
bool AutostartFlow::waitJoinConfirm(autostartStep next, uint8_t joinConfirmWaitTime) {
    modemInterpreterReff.answerStepwiseOnIncommingMessages();
    return getTimeOutStatusAndSetFlowStep(joinConfirmWaitTime, next);
}

void AutostartFlow::forceJoinNetwork(autostartStep next) {
    setPlcFlowState(tPlcFlowState::forceJoining);
    if (ProgramConfig::isCoordinator()) {
        ModemController::restoreMemberList();
        plcAutoStart = ReloadCoordVerifyStart;
        return;
    }
    // else Peer
    plcAutoStart = next;
    const SimpleFlashEditor::Parameter_A &sys_param =
            SimpleFlashEditor::getFlashA();
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtStartForceJoin),
            sys_param.targetNetId, sys_param.ownNetworkAddress);
    termFilterPtr_->println(Global2::OutBuff);
    ModemController::triggerSinglePlcRequestWithArgs(
            PLC_CommandTypeSpecification::forceJoinNetwork, 3, channel0,
            sys_param.targetNetId, sys_param.ownNetworkAddress);
}

/***
 * Step to Proof whether the join was successful or not
 * If the join fails the step chain transit  to search or join
 * again
 */
void AutostartFlow::profJoinConnection(autostartStep joinStep,
        autostartStep retryStep, autostartStep failStep) {
    static uint8_t joinCallOut = 0;
    modemInterpreterReff.answerStepwiseOnIncommingMessages();
    if (ModemController::isConnectedToCoordinator()) {
        plcAutoStart = joinStep;    // announceFinishStep;
        set_proceed_again();
    } else {
        termFilterPtr_->println(TX::cTxtJoinFailed);
        if (joinCallOut++ > 3) {
            joinCallOut = 0;
            plcAutoStart = failStep;    // deleteNetworkAgentStep;
        } else {
            termFilterPtr_->println(TX::cTxtWaitNextTry);
            plcAutoStart = retryStep;
        }
    }
}

void AutostartFlow::waitAfterJoinFail() {
    uint16_t waitTime = 4;
    bool waitTimeout = getTimeOutStatusAndSetFlowStep(waitTime, joinNetworkStep);
    if (waitTimeout) {
        termFilterPtr_->println(TX::cTxtRestartJoin);
    } else {
        termFilterPtr_->putchar('#');
    }
}


void AutostartFlow::announceFinish() {
    ModemController::setRequestRepeatMode(false);
    termFilterPtr_->println(TX::cTxtPLCAutoStartFin);
    autostartFinishedStatus = true;
    int job = TimeMeasurement::getJobIndex(*ProgramConfig::getRunTimeJobPtr());
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtStartTimeMsg),
            TimeMeasurement::toString((uint8_t) job, tmp, sizeof(tmp), TimeMeasurement::TimeHHMMSSms));
    termFilterPtr_->println(Global2::OutBuff);
    setPlcFlowState(tPlcFlowState::done);
    ledFlowReff_.setLedMode(LEDSignalFlow::connected);
    plcAutoStart = runTests;
}

/***
 * This step is needed in Order to cycle throw all network agent possibilities offered by the network search request
 */
void AutostartFlow::deleteNetworkAgent() {
    termFilterPtr_->println(TX::cTxtDelAgentMsg);
    bestFoundDescriptorPtr->panId = 0;
    plcAutoStart = proofSearchResultsStep;
}

void AutostartFlow::resetSoftwareForDefaultSearch() {
    SimpleFlashEditor::setAutoStartFlowMode(0);
    MicrocontrollerConfig::softwareReset();
}

/***
 * This method is used to wait for certain timeouts
 * @param waitTime => Wait time after the timeout triggers
 * @param transitionStep => next step in the plcAutoStart step chain
 * @return timeout trigger status: false => no trigger // true  => timeout triggered
 */
bool AutostartFlow::getTimeOutStatusAndSetFlowStep(uint16_t waitTime, autostartStep transitionStep) {
    static uint16_t waitCount = 0;
    static uint8_t lastStep = (uint8_t) transitionStep;
    if ((lastStep != transitionStep) || (waitTime == 0)) {
        lastStep = transitionStep;
        waitCount = 0;
    }
    if (++waitCount >= waitTime) {
        plcAutoStart = transitionStep;
        set_proceed_again();
        return true;
    }
    return false;
}

// This function checks, if the force join tables are available
// Return: true: Force Join may be executed
bool AutostartFlow::checkForceJoinTabs() {
    IKeyContainerStorage::container attributeContainer;
    IKeyContainerStorage *keyValueStore = StorageAdministrator::getStorageInterfacePtr();
    IKeyContainerStorage::skey temp_key;

    const SimpleFlashEditor::Parameter_A &sys_param = SimpleFlashEditor::getFlashA();
    if (sys_param.ownNetworkAddress > ModemController::networkMemberMax) {
        return false;
    }

    // Is 1st entry in the particular table available?
    struct check_obj {
        bool coordinatorOnly;
        uint8_t idx;
        uint16_t table;
    };
    const check_obj check_keys[] = { { false, 0,
            StorageAdministrator::adpRouteTable }, { false, 0,
            StorageAdministrator::macNeighbourTable }, { false, 0,
            StorageAdministrator::frameCounter }, { true, 1,
            StorageAdministrator::clientInfoTable } };
    const uint32_t len = sizeof(check_keys) / sizeof(check_obj);
    for (uint32_t i = 0; i < len; i++) {
        const check_obj &ck = check_keys[i];
        if (!ProgramConfig::isCoordinator() && ck.coordinatorOnly) {
            continue;
        }
        temp_key.extended_key.key = (uint8_t) ck.table;
        temp_key.extended_key.index = ck.idx;
        keyValueStore->loadContainerWithKey(&temp_key, &attributeContainer);
        if (attributeContainer.data == NULL) {
            return false;
        }
    }

    return true;
}

// Send a message to the coordinator to check the force join.
void AutostartFlow::ReloadSendP2CVerificationMsg() {
    modemInterpreterReff.setForceJoinState(ModemInterpreter::FJpending);
    termFilterPtr_->print(TX::cTxtForceJoinCheckMsg);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%cR%04d %04d", ModemInterpreter::commandPrefix,
            ModemInterpreter::ForceJoinCheckRequest,
             ModemController::getMacShortAddress(),
             ModemController::getNetworkIPAddress());
    ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
            reinterpret_cast<uint8_t*>(Global2::OutBuff), 13, IRoutingManager::BCGlobalIP, true,
            TermContextFilter::filterForceJoinInfo);
    modemInterpreterReff.setInterpretState(
            ModemInterpreter::proofForceJoinVerification);
    ModemInterpreter::setDataStatusSet(false);
}

// Waits for an answer, which is sent with by routine ReloadSendP2CVerificationMsg.
// If the answer is available the next step is registered.
// True: calling routine may change the step
bool AutostartFlow::ReloadWaitP2CVerification() {
    // termFilterPtr_->putchar('$');
    modemInterpreterReff.answerStepwiseOnIncommingMessages();
    ModemInterpreter::sForceJoinState state = modemInterpreterReff.getForceJoinState();
    // sprintf(Global2::OutBuff," FJ-State:%x",state);
    // termFilterPtr_->print(Global2::OutBuff);

    if ((state & ModemInterpreter::FJconfirmed) || (state & ModemInterpreter::c2pIPReceived)) {
        plcAutoStart = announceFinishStep;
        return false;
    }
    uint32_t timeout = SimpleFlashEditor::getFJDelay(SimpleFlashEditor::fjDelayX);
    if (state & ModemInterpreter::c2pBCReceived) {
        timeout += SimpleFlashEditor::getFJDelay(SimpleFlashEditor::fjDelayY);
    }
    if (FJPeerActTime > timeout) {
        if ((state & ModemInterpreter::FJtimeout) == 0) {
            termFilterPtr_->print(TX::ForceJoinTimeoutMsg);
            modemInterpreterReff.setForceJoinState(ModemInterpreter::FJtimeout);
        } else {
            FlowMode1AndResetProcess();
        }
    }
    return true;
}

void AutostartFlow::FlowMode1AndResetProcess() {
#ifdef USE_preset_Search_Mode
    ModemController::triggerSinglePlcRequestWithArgs(leaveNetwork, 1, channel0);
    presetSearchMode = true;
#else
    // Activate standard Network search
    uint16_t mode = SimpleFlashEditor::getAutoStartFlowMode();
    mode &= (uint16_t) ~SimpleFlashEditor::ForceJoinMode;
    SimpleFlashEditor::setAutoStartFlowMode(mode);

    termFilterPtr_->println(TX::SoftwareResetMsg);
    for (int i = 0; i < 100000; i++) {
        termFilterPtr_->popTxD();
    }
    MicrocontrollerConfig::softwareReset();
    while (1) {
        asm("nop");
    }
#endif
}

// Sends a Message from coordinator to peer to check the ForceJoin.
// @return: true: message send. false: no message send.
bool AutostartFlow::ReloadSendC2PVerificationMsg(uint16_t peerIdx) {
    NetworkMember *pn = ModemController::getNetMemberPtrByIndex(peerIdx);
    unsigned int ipAdr;
    if (pn != NULL && pn->getLink() == NetworkMember::notVerified) {
        ipAdr = pn->getNetworkIPAddress();
        modemInterpreterReff.setForceJoinState(ModemInterpreter::FJpending);
#if debug_level_local > 0
        if (termFilterPtr_->isNotFiltered(TermContextFilter::filterForceJoinS)) {
            termFilterPtr_->print(TX::cTxtForceJoinCheckMsg);
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, ", IP:%u", ipAdr);
            termFilterPtr_->print(Global2::OutBuff);
        }
#endif
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%cC%04d", ModemInterpreter::commandPrefix,
                ModemInterpreter::ForceJoinCheckRequest, ipAdr);
        ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                reinterpret_cast<uint8_t*>(Global2::OutBuff), 8, IRoutingManager::BCGlobalIP,
                true, TermContextFilter::filterForceJoinInfo);
        modemInterpreterReff.setInterpretState(ModemInterpreter::proofForceJoinVerification);
        ModemInterpreter::setDataStatusSet(false);
        return true;
    }
    return false;
}

/***
// Warte auf die Antwort, die bei ReloadSendC2PVerificationMsg versendet wird und richte den nächsten Schritt ein
void AutostartFlow::ReloadWaitC2PVerification() {
	termFilterPtr_->putchar('$');
	modemInterpreterReff.answerStepwiseOnIncommingMessages();
	ModemInterpreter::sForceJoinState state = modemInterpreterReff.getForceJoinState();
	if (!getTimeOutStatusAndSetFlowStep(60,initModemStep)) {	// ToDo: 60 <-> Timeout aus Paramersatzverwenden
		switch (state) {
			case ModemInterpreter::timeout:
			case ModemInterpreter::error:
				CheckFail = true;
			// no break;
			case ModemInterpreter::confirmed:
				plcAutoStart = ReloadCoordInitDelay;
		}
	} else {
		modemInterpreterReff.setForceJoinState(ModemInterpreter::timeout);
		plcAutoStart = ReloadCoordInitDelay;
	}
}
***/

// Check if there are peers with the "notVerified" link state.
// Return: false: at least 1 peer is in the "notVerified" state.
bool AutostartFlow::ReloadCoordCheckPeers() {
    bool result = true;
    NetworkMember *pn;
    for (uint32_t i = 1; i <= ModemController::getNetworkMemberMax(); i++) {
        pn = ModemController::getNetMemberPtrByIndex((uint16_t) i);
        if (pn != NULL) {
            NetworkMember::linkStatus link = pn->getLink();
            if (link == NetworkMember::notVerified) {
                result = false;
                break;
            }
        }
    }
    return result;
}

// Remove peers who did not response.
void AutostartFlow::ReloadCoordDelPeers() {
    List<PlcCommand> &ref = ModemController::disableNotConnectedPeers();
    if (!ref.isEmpty()) {
        ModemController::setControllerState(
                ModemController::forceJoinCoordDelPeer);
        ModemController::setProcessState(ref.getHead());
    }
//  NetworkMember* pn;
//  for (uint32_t i=1;i<=ModemController::getNetworkMemberMax();i++) {
//      pn = ModemController::getNetMemberPtrByIndex((uint16_t) i);
//      if (pn != NULL) {  // && pn->getOutgoingRoute()
//          NetworkMember::linkStatus link = pn->getLink();
//          if (link == NetworkMember::notVerified) {
//             ModemController::triggerSinglePlcRequestWithArgs(PLC_CommandTypeSpecification::setClientInfo,
//                              4, channel0, false, pn->getMacShortAddress(), (uint32_t)pn->getMacAddress());
//             // pn->setLink(NetworkMember::disconnected)
//             ModemController::disableNetMemberAtIndex((uint16_t) i, false);
//          plcAutoStart = proofSearchResultsStep;
//          }
//      }
//  }
}

char* AutostartFlow::setFJIgnoreRequests(char *text) {
    const char *txt = charOperations::skip_token(text, ' ');
    char *member;
    unsigned int nTimes, cnt;
    cnt = sscanf(txt, "%u", &nTimes);
    if (cnt == 1) {
        ignoreNTimesFJRequests = (uint8_t) nTimes;
    } else {
        nTimes = ignoreNTimesFJRequests;
    }

    if (ProgramConfig::isCoordinator()) {
        member = const_cast<char*>(TX::getText(TX::txtPeer));
    } else {
        member = const_cast<char*>(TX::getText(TX::ShowCoordinator));
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength ,
            TX::getText(TX::IgnoreForceJoinNTimes), member, nTimes);
    return Global2::OutBuff;
}

void AutostartFlow::restartTestSuite() {
    if (plcAutoStart == deactivated) {
        plcAutoStart = runTests;
    }
}

}  // namespace AME_SRC
