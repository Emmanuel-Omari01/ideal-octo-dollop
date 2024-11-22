/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */
#include "../../../../Renesas/Generated/r_flash_rx/src/targets/rx111/r_flash_rx111.h"

#include "UpdateModuleTests.h"

#include "UnitSuiteTests.h"
#include "../../SystemBase/ProgramConfig.h"
#include "../../UpdateModule/CodeFlashEditor.h"
#include "../../UpdateModule/IUpdateTranslator.h"
#include "../../UpdateModule/PlcUpdateTranslator.h"
#include "../../UpdateModule/SoftwareDeploymentFlow.h"
#include "../../UpdateModule/UpdateStorageController.h"
#include "../../SystemLogic/SystemStateController.h"
#include "../TestSupport/OutputAnalyzer.h"
#include "../../../../OpenSource/uCUnit/uCUnit-v1.0.h"
#include "../../SystemLogic/SystemStateController.h"

namespace AME_SRC {

UpdateModuleTests::UpdateModuleTests(TermContextFilter *term) {
    termFilterPtr_ = term;
}

UpdateModuleTests::~UpdateModuleTests() {
}

/*
 * This test proof if the updateTranslator can translate update
 * control commands
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test018TranslatePlcStartUpdate() {
    UCUNIT_TestcaseBegin("018 UpdateModule translate PLC start update");
    // Arrange
    PlcUpdateTranslator plcTranslator;
    IUpdateTranslator *translatorPtr = &plcTranslator;
    const char *message = "u:s:01:00:000:00000";

    // Act
    auto uMessage = translatorPtr->translateMessage(message);
    bool isStartType = (uMessage.getType() == UpdateType::start);
    bool isUpdate = translatorPtr->isUpdateMessage();

    // Assert
    UCUNIT_CheckIsEqual(isStartType, true);
    UCUNIT_CheckIsEqual(isUpdate, true);
    UCUNIT_TestcaseEnd();
}

/*
 * This test proof if the updateTranslator can translate an update
 * string with a certain Format.
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test019TranslateUFragment() {
    UCUNIT_TestcaseBegin("019 UpdateModule translate update fragment");
    // Arrange
    PlcUpdateTranslator plcTranslator;
    IUpdateTranslator *translatorPtr = &plcTranslator;
    const char *message = "u:f:59:20:100:0A0B0C";

    // Act
    auto uMessage = translatorPtr->translateMessage(message);

    // Assert
    UCUNIT_CheckIsEqual(uMessage.getType(), UpdateType::fragment);
    UCUNIT_CheckIsEqual(uMessage.getId(), 59);
    UCUNIT_CheckIsEqual(uMessage.getOffset(), 20);
    UCUNIT_CheckIsEqual(uMessage.getSize(), 100);
    UCUNIT_TestcaseEnd();
}

/*
 * This test proof if the updateTranslator detects wrong Message Types
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test020TranslateWrongType() {
    UCUNIT_TestcaseBegin("020 UpdateModule translate wrong type");
    // Arrange
    PlcUpdateTranslator plcTranslator;
    IUpdateTranslator *translatorPtr = &plcTranslator;
    const char *message = "u:k:59:20:100:0A0B0C";

    // Act
    auto uMessage = translatorPtr->translateMessage(message);

    // Assert
    UCUNIT_CheckIsEqual(uMessage.getType(), UpdateType::error);
    UCUNIT_CheckIsEqual(uMessage.getId(), 0);
    UCUNIT_CheckIsEqual(uMessage.getOffset(), 0);
    UCUNIT_CheckIsEqual(uMessage.getSize(), 0);
    UCUNIT_TestcaseEnd();
}

/*
 * This test proof if the updateTranslator queue can be accessed after a
 * Message is stored.
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test021TranslatorReStoredMessage() {
    UCUNIT_TestcaseBegin("021 UpdateModule restore data fragment");
    // Arrange
    PlcUpdateTranslator plcTranslator;
    IUpdateTranslator *translatorPtr = &plcTranslator;
    uint8_t machineCode[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    UpdateMessage uInputMessage = { UpdateType::fragment, 59, 20, 100,
            machineCode };

    // Act
    translatorPtr->storeMessage(uInputMessage);
    auto uOutputMessage = translatorPtr->getNextTranslation();

    // Assert
    UCUNIT_CheckIsEqual(uInputMessage.getId(), uOutputMessage.getId());
    UCUNIT_CheckIsEqual(uInputMessage.getType(), uOutputMessage.getType());
    UCUNIT_CheckIsEqual(uInputMessage.getOffset(), uOutputMessage.getOffset());
    UCUNIT_CheckIsEqual(uInputMessage.getSize(), uOutputMessage.getSize());
    UCUNIT_TestcaseEnd();
}

/*
 * This test proof if the updateTranslator store message in fifo order.
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test022TranslatorReStoreFifo() {
    UCUNIT_TestcaseBegin("022 UpdateModule restore fifo");
    // Arrange
    PlcUpdateTranslator plcTranslator;
    IUpdateTranslator *translatorPtr = &plcTranslator;
    uint8_t machineCode[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    UpdateMessage uInputMessage1 = { UpdateType::fragment, 59, 20, 100,
            machineCode };
    UpdateMessage uInputMessage2 = { UpdateType::fragment, 60, 20, 100,
            machineCode };

    // Act
    translatorPtr->storeMessage(uInputMessage1);
    translatorPtr->storeMessage(uInputMessage2);
    auto uOutputMessage = translatorPtr->getNextTranslation();

    // Assert
    UCUNIT_CheckIsEqual(uInputMessage1.getId(), uOutputMessage.getId());
    UCUNIT_CheckIsEqual(uInputMessage1.getType(), uOutputMessage.getType());
    UCUNIT_CheckIsEqual(uInputMessage1.getOffset(), uOutputMessage.getOffset());
    UCUNIT_CheckIsEqual(uInputMessage1.getSize(), uOutputMessage.getSize());
    UCUNIT_TestcaseEnd();
}

/*
 * This test proof if the update-message-queue keep the same order in
 * case of an overflow
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test023TranslatorReStoreFifoWhenOverflow() {
    UCUNIT_TestcaseBegin("023 UpdateModule restore fifo when overflow");
    // Arrange
    PlcUpdateTranslator plcTranslator;
    IUpdateTranslator *translatorPtr = &plcTranslator;
    uint8_t machineCode[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    UpdateMessage uInputMessage1 = { UpdateType::fragment, 59, 20, 100,
            machineCode };

    // Act
    for (int i = 1; i < 11; i++) {
        uInputMessage1.setId(i);
        translatorPtr->storeMessage(uInputMessage1);
    }
    auto uOutputMessage1 = translatorPtr->getNextTranslation();
    for (int i = 1; i < 9; i++) {
        translatorPtr->getNextTranslation();
    }
    auto uOutputMessage2 = translatorPtr->getNextTranslation();

    // Assert
    UCUNIT_CheckIsEqual(1, uOutputMessage1.getId());
    UCUNIT_CheckIsEqual(10, uOutputMessage2.getId());
    UCUNIT_TestcaseEnd();
}

/*
 * This test proof if an Update-Message can be stored in code flash.
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test024StorageControlLocalFlashSave() {
    UCUNIT_TestcaseBegin(
            "024 UpdateModule save update message permanent in local system");

    // Arrange
    uint8_t machineCode[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    UpdateMessage uMessage = { UpdateType::fragment, 59, 20, 100, machineCode };
    uint8_t *assertPointer = reinterpret_cast<uint8_t*>(FLASH_CF_BLOCK_1);
    CodeFlashEditor cFlashEditor;
    IStorage *localStoragePtr = &cFlashEditor;
    UpdateStorageController uStorageControl(localStoragePtr);
    static int i = 0;
    int calculatedCRC = 0;
    int eraseMeasurement = 0;
    int n = sizeof(machineCode) / sizeof(uint8_t);
    int expectedCRC = n * (n + 1) / 2;  // Sum formula Gauß
    int expectedEraseValue = n * 0xFF;
    cFlashEditor.setup();

    // Act
    localStoragePtr->clear(FLASH_CF_BLOCK_1, 1);
    eraseMeasurement = sumFieldValues(assertPointer, n);
    if (cFlashEditor.isReady()) {
        uStorageControl.storeFragment(uMessage, FLASH_CF_BLOCK_1);
    }
    calculatedCRC = sumFieldValues(assertPointer, n);

    // Assert
    UCUNIT_CheckIsEqual(expectedEraseValue, eraseMeasurement);
    UCUNIT_CheckIsEqual(expectedCRC, calculatedCRC);
    UCUNIT_TestcaseEnd();
}

/*
 * This is a helper function to sum up field values
 */
int UpdateModuleTests::sumFieldValues(uint8_t *fieldPtr, uint8_t size) {
    int sum = 0;
    for (int j = 0; j < size; j++) {
        sum += *(fieldPtr + j);
    }
    return sum;
}

/*
 * This test proofs whether an Update-Message which is out of valid memory boundary
 * is detected.
 */
#pragma GCC diagnostic ignored "-Wstack-usage="
void UpdateModuleTests::test025StorageControlInvalidBaseAddress() {
    const uint32_t FLASH_CF_BLOCK_511 = 0xFFF80000;
    const uint32_t FLASH_CF_BLOCK_0 = 0xFFFFFC00;
    UCUNIT_TestcaseBegin(
            "025 UpdateModule save update message with wrong base address");

    // Arrange
    uint8_t machineCode[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    UpdateMessage uMessage = { UpdateType::fragment, 59, 20, 100, machineCode };
    IStorage::storeStates measuredStoreStatusLow;
    IStorage::storeStates measuredStoreStatusHigh;
    uint32_t baseAddressLow = (FLASH_CF_BLOCK_511 - 0xFF);
    uint32_t baseAddressHigh = (FLASH_CF_BLOCK_0 + 0xF);
    CodeFlashEditor cFlashEditor;
    IStorage *localStoragePtr = &cFlashEditor;
    UpdateStorageController uStorageControl(localStoragePtr);
    cFlashEditor.setup();

    // Act
    uStorageControl.storeFragment(uMessage, baseAddressLow);
    measuredStoreStatusLow = uStorageControl.getStatus();

    uStorageControl.storeFragment(uMessage, measuredStoreStatusHigh);
    measuredStoreStatusHigh = uStorageControl.getStatus();

    // Assert
    UCUNIT_CheckIsEqual(IStorage::kFailure, measuredStoreStatusLow);
    UCUNIT_CheckIsEqual(IStorage::kFailure, measuredStoreStatusHigh);
    UCUNIT_TestcaseEnd();
}

/*
 * This test proofs if the DeployFlow starts.
 */
void UpdateModuleTests::test015StartUpdateDeployProcess() {
    UCUNIT_TestcaseBegin("015 Start UpdateDeploy Process");
    uint8_t udpHeaderOffset = 16;
    enum testSteps {
        act, actWait, assert
    };
    OutputAnalyzer outAnalyzer;
    DelayHandler delyHandler;
    IEventStateProvider* stateControll = new SystemStateController();
    SoftwareDeploymentFlow deployFlow(
            ProgramConfig::getPlcModemPtr()->getModemControllPtr(), 0,
            delyHandler, 0xFF, stateControll);
    static TermContextFilter::filterBits filter;
    static testSteps step = act;
    const uint8_t kMessageLength = 8;
    const char *expectedStartMessage = "::U:S";
    bool isEqual = false;

    switch (step) {
    case act:
        filter = TermContextFilter::getFilterMask();
        TermContextFilter::setFilterMask(TermContextFilter::filterPLCOutput);
        UnitSuiteTests::setProcessing(true);
        deployFlow.start();
        deployFlow.proceed();
        deployFlow.stop();
        step = actWait;
        break;
    case actWait:
        step = assert;
        break;
    case assert:
        isEqual = outAnalyzer.isGlobalOutputEqualTo(expectedStartMessage);
        TermContextFilter::setFilterMask(filter);
        UCUNIT_CheckIsEqual(true, isEqual);
        UCUNIT_TestcaseEnd();
        step = act;
        UnitSuiteTests::setProcessing(false);
        break;
    }
}

/*
 * This test proofs if a Fragment is repeated after a negative
 * Acknowledge is received
 */
void UpdateModuleTests::test016RepeatUpdateAfterNack() {
    UCUNIT_TestcaseBegin("016 Repeat Update Fragment After Nack");
    // Arrange
    DelayHandler delyHandler;
    IEventStateProvider* stateControll = new SystemStateController();
    SoftwareDeploymentFlow deployFlow(
            ProgramConfig::getPlcModemPtr()->getModemControllPtr(), 0,
            delyHandler, 0xFF, stateControll);
    PlcUpdateTranslator translator;
    const uint8_t kMessageLength = 19;
    const char *recivedNackReply = "::u:n:005:002:1024:";
    const char *expectedRepeat = "::U:f:005:002:1024:";
    char generatedReply[kMessageLength];
    bool isEqual = false;
    deployFlow.setFlowState(SoftwareDeploymentFlow::kPollCheckAnswer);
    UpdateMessage nackMessage = translator.translateMessage(recivedNackReply);

    // Act
    translator.storeMessage(nackMessage);
    deployFlow.proceed();

    // Assert
    snprintf(generatedReply, kMessageLength, Global2::OutBuff);
    isEqual = (strcmp(generatedReply, expectedRepeat) == 0);
    UCUNIT_CheckIsEqual(true, isEqual);
    UCUNIT_TestcaseEnd();
}

/*
 * This test proofs if the update start call is blocked during
 * event mode on coordinator side.
 */
void UpdateModuleTests::test010BlockUpdateDuringEventMode() {
    UCUNIT_TestcaseBegin("010 Block Update During Coordinator Event Mode");
    // Arrange
    DelayHandler delyHandler;
    IEventStateProvider* stateControll = new SystemStateController();
    SoftwareDeploymentFlow deployFlow(
            ProgramConfig::getPlcModemPtr()->getModemControllPtr(), 0,
            delyHandler, 0xFF, stateControll);
    SoftwareDeploymentFlow::deploySteps deployStepAck =
            SoftwareDeploymentFlow::kIdle;
    SoftwareDeploymentFlow::deploySteps deployStepNack =
            SoftwareDeploymentFlow::kIdle;
    bool cordStatus = ProgramConfig::isCoordinator();
    ProgramConfig::setCoordinatorStatus(true);

    // Act
    stateControll->setState(IEventStateProvider::kIdle);
    deployFlow.start();
    deployStepAck = deployFlow.getDeployStep();
    deployFlow.stop();

    stateControll->setState(IEventStateProvider::kImportantEvent);
    deployFlow.start();
    deployStepNack = deployFlow.getDeployStep();
    deployFlow.stop();

    // Assert
    UCUNIT_CheckIsEqual(deployStepAck, SoftwareDeploymentFlow::kStart);
    UCUNIT_CheckIsEqual(deployStepNack, SoftwareDeploymentFlow::kIdle);
    ProgramConfig::setCoordinatorStatus(cordStatus);
    UCUNIT_TestcaseEnd();
}

/*
 * This test proofs if update fragment processing is blocked
 * on peer side during monitoring mode.
 */
void UpdateModuleTests::test013BlockUpdateIfNotProcessingUpdate() {
    UCUNIT_TestcaseBegin("013 Block Update If Peer Is Not Updatting");
    // Arrange
    DelayHandler delyHandler;
    IEventStateProvider* stateControll = new SystemStateController();
    SoftwareDeploymentFlow deployFlow(
            ProgramConfig::getPlcModemPtr()->getModemControllPtr(), 0,
            delyHandler, 0xFF, stateControll);
    IUpdateTranslator *translatorPtr = new PlcUpdateTranslator();
    SoftwareDeploymentFlow::deploySteps deployStepAck =
            SoftwareDeploymentFlow::kIdle;
    SoftwareDeploymentFlow::deploySteps deployStepNack =
            SoftwareDeploymentFlow::kIdle;
    const char *recivedFragment = "::u:n:005:002:1024:";
    UpdateMessage fragMessage = translatorPtr->translateMessage(
            recivedFragment);
    bool cordStatus = ProgramConfig::isCoordinator();
    ProgramConfig::setCoordinatorStatus(false);

    // Act
    stateControll->setState(IEventStateProvider::kIdle);
    translatorPtr->storeMessage(fragMessage);
    deployFlow.proceed();
    deployStepAck = deployFlow.getDeployStep();

    stateControll->setState(IEventStateProvider::kImportantEvent);
    translatorPtr->storeMessage(fragMessage);
    deployFlow.proceed();
    deployStepNack = deployFlow.getDeployStep();

    // Assert
    UCUNIT_CheckIsEqual(deployStepAck, SoftwareDeploymentFlow::kProcessFragment);
    UCUNIT_CheckIsEqual(deployStepNack, SoftwareDeploymentFlow::kIdle);
    ProgramConfig::setCoordinatorStatus(cordStatus);
    UCUNIT_TestcaseEnd();

    delete translatorPtr;
}

} /* namespace AME_SRC */
