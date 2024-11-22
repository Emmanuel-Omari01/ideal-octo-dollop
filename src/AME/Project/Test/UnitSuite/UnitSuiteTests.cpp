/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */
#include "UnitSuiteTests.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "../../../Project/PlcCommunication/AliveHandling/MessageForwarding/BroadcastRoutingManager.h"
#include "../../../Project/Terminal/SerialProfiles/SerialStandardProfile.h"
#include "../../../Project/SystemBase/ProgramConfig.h"
#include "../../../Project/HelpStructures/DelayHandler.h"
#include "../../../Project/SignalProcessing/I2C/I2cApp.h"
extern "C" {
#include "../../../../OpenSource/uCUnit/uCUnit-v1.0.h"
}
#include "../TestSupport/OutputAnalyzer.h"


extern "C" uint32_t R_RSPI_GetVersion(void);    // see "../../../../Renesas/Generated/r_rspi_rx/r_rspi_rx_if.h"

namespace AME_SRC {

bool UnitSuiteTests::processing = false;
TermContextFilter *UnitSuiteTests::termFilterPtr_;

UnitSuiteTests::UnitSuiteTests(AME_SRC::SerialDriver *term) :
        testsFinished(false), testsEnabled(false), testStep(0), testParameter(0) {
    this->term_ptr = term;
    termFilterPtr_ = new TermContextFilter(term);
    termFilterPtr_->setContext(TermContextFilter::filterUnitTest);
    pUpdateModuleTests = new UpdateModuleTests(termFilterPtr_);
}

UnitSuiteTests::~UnitSuiteTests() = default;


void UnitSuiteTests::setLastUnitTest(bool set) {
    UCUnit_setLastTest(set);
}
bool UnitSuiteTests::getLastUnitTest() {
    return UCUnit_getLastTest();
}


// Entry to run tests of UnitSuiteTests.
// runTests is called with an interval of 1s as soon as modem is initialized
void UnitSuiteTests::runTests() {
    if (!testsEnabled) {
        return;
    }
    switch (testParameter) {
    case 0:
        // Free test slot can be used for tests
        break;
    case 1:
        test001RoleLineAssignment();
        break;
    case 2:
        // Free test slot can be used for tests
        break;
    case 3:
        test003RouteIdIsChecked();
        break;
    case 4:
        test004AliveMessageNodeIncrement();
        break;
    case 5:
        // Free test slot can be used for tests
        break;
    case 6:
        test006AliveMappingDisplay();
        break;
    case 7:
        // Free test slot can be used for tests
        break;
    case 8:
        test008ProofSPIDriver();
        break;
    case 9:
        // Free test slot can be used for tests
        break;
    case 10:
        pUpdateModuleTests->test010BlockUpdateDuringEventMode();
        break;
    case 11:
        test011ProofNetworkIsCleaved();
        break;
    case 12:
        test012ProofNetworkIsNotCleaved();
        break;
    case 13:
        pUpdateModuleTests->test013BlockUpdateIfNotProcessingUpdate();
        break;
    case 14:
        test014MSTBigSystem();
        break;
    case 15:
        pUpdateModuleTests->test015StartUpdateDeployProcess();
        break;
    case 16:
        pUpdateModuleTests->test016RepeatUpdateAfterNack();
        break;
    case 17:
        test017IOInputRequestHandling();
        break;
    case 18:
        pUpdateModuleTests->test018TranslatePlcStartUpdate();
        break;
    case 19:
        pUpdateModuleTests->test019TranslateUFragment();
        break;
    case 20:
        pUpdateModuleTests->test020TranslateWrongType();
        break;
    case 21:
        pUpdateModuleTests->test021TranslatorReStoredMessage();
        break;
    case 22:
        pUpdateModuleTests->test022TranslatorReStoreFifo();
        break;
    case 23:
        pUpdateModuleTests->test023TranslatorReStoreFifoWhenOverflow();
        break;
    case 24:
        pUpdateModuleTests->test024StorageControlLocalFlashSave();
        break;
    case 25:
        pUpdateModuleTests->test025StorageControlInvalidBaseAddress();
        break;
    case 28:
        test028FilteringOutputs();
        break;
    case 29:
        test029MacViaI2c();
        break;
    case 30:
        // blocked number. #30 is used for first integration test
        break;
    case 31:
        test031InitHardware();
        break;
    }

    if (!processing) {
        testsFinished = true;
        testStep = 0;
        if (UCUnit_getLastTest()) {
            // signal build server that all tests are finished
            termFilterPtr_->putchar(SerialStandardProfile::EOT);
        }
        termFilterPtr_->setFilterStandard();
    }
}  // runTests

uint8_t* UnitSuiteTests::tsrd_prepare_stream(uint8_t *stream, uint32_t streamLen, uint8_t ip, uint8_t count) {
    snprintf(reinterpret_cast<char *>(stream), streamLen, "::SD%04d ", ip);
    stream += 9;
    *stream++ = count;
    return stream;
}

uint8_t* UnitSuiteTests::tsrd_add_stream(uint8_t *stream, uint16_t dest_ip,
        uint8_t linkQuality) {
    uint16_t *pip = reinterpret_cast<uint16_t*>(stream);
    *pip = dest_ip;
    stream += 2;
    *stream++ = linkQuality;
    return stream;
}

void UnitSuiteTests::fakeGivenAmountOfMemberIps(uint8_t fakeAmount,
        bool &hasFinished) {
    static const uint16_t ips[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    static uint8_t fakeIndex = 0;
    if (fakeIndex++ < fakeAmount) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "forceNetMember %u %u %u 0", ips[fakeIndex],
                ips[fakeIndex], ips[fakeIndex]);
        term_ptr->set_buffer(1, Global2::OutBuff);
    } else {
        hasFinished = true;
        fakeIndex = 0;
    }
}

// This test collection proof the role distribution among peers in
// a network arranged as a line
void UnitSuiteTests::test001RoleLineAssignment() {
    enum assignmentTestSteps {
        testSetup,
        testProofCoord,
        testProofP1,
        testProofP2,
        testProofP3,
        testEnd
    };
    static assignmentTestSteps step = testSetup;
    IRoutingManager::MemberIpTupel *roles;
    switch (step) {
    case testSetup:
        processing = true;
        UCUNIT_TestcaseBegin("001 Role assignment in line");
        ModemController::activateTestMembersUptoIP(3);
        compArranger.create3PeerLine();
        AssignmentFlow::adjazenzParentPtr_->fillMatrixFromDiscoveryData(false);
        AssignmentFlow::storeDataForBypassingEOBIntersection();
        AssignmentFlow::printEOBBypassingResults();

        step = testProofCoord;
        break;
    case testProofCoord: {
        roles = AssignmentFlow::getCommunicationRolesForPeerIP(0);
        UCUNIT_WriteString("proof coordinator:\n");
        testRoutingData testData[5] = { { 0, IRoutingManager::Administrator,
                IRoutingManager::BGEnd },
                { 1, IRoutingManager::Transmitter, 1 }, { 2,
                        IRoutingManager::Supervisor, 2 }, { 3,
                        IRoutingManager::notUsedID, 0 }, { 4,
                        IRoutingManager::ListenerB1, 3 }, };
        checkRolesWithRoutingDataField(roles, testData, 3);
        step = testProofP1;
        break;
    }
    case testProofP1: {
        roles = AssignmentFlow::getCommunicationRolesForPeerIP(1);
        UCUNIT_WriteString("proof peer1:\n");
        testRoutingData testData[5] = { { 0, IRoutingManager::Receiver, 1 }, {
                1, IRoutingManager::Transmitter, 2 }, { 2,
                IRoutingManager::Supervisor, 3 }, { 3,
                IRoutingManager::notUsedID, 0 }, { 4,
                IRoutingManager::ListenerB1, IRoutingManager::BGEnd }, };
        checkRolesWithRoutingDataField(roles, testData, 3);
        step = testProofP2;
        break;
    }
    case testProofP2: {
        roles = AssignmentFlow::getCommunicationRolesForPeerIP(2);
        UCUNIT_WriteString("proof peer2:\n");
        testRoutingData testData[5] = { { 0, IRoutingManager::Receiver, 2 }, {
                1, IRoutingManager::Transmitter, 3 }, { 2,
                IRoutingManager::Supervisor, IRoutingManager::BGEnd }, { 3,
                                                                        IRoutingManager::ListenerF1, 1 }, { 4,
                IRoutingManager::notUsedID, 0 }, };
        checkRolesWithRoutingDataField(roles, testData, 3);
        step = testProofP3;
        break;
    }
    case testProofP3: {
        roles = AssignmentFlow::getCommunicationRolesForPeerIP(3);
        UCUNIT_WriteString("proof peer3:\n");
        testRoutingData testData[5] = { { 0, IRoutingManager::Receiver, 3 },
                                        { 1, IRoutingManager::Transmitter, IRoutingManager::BGEnd },
                                        { 2, IRoutingManager::Supervisor, IRoutingManager::BGEndAck },
                                        { 3, IRoutingManager::ListenerF1, 2 },
                                        { 4, IRoutingManager::notUsedID, 0 }, };
        checkRolesWithRoutingDataField(roles, testData, 3);
        step = testEnd;
        break;
    }
    case testEnd:
        processing = false;
        UCUNIT_TestcaseEnd();
        step = testSetup;
        break;
    }
}

// This test proof if round trip IDs are checked during interrupt handling
#pragma GCC diagnostic ignored "-Wstack-usage="
void UnitSuiteTests::test003RouteIdIsChecked() {
    UCUNIT_TestcaseBegin(
            "003 Proof routing ID is checked during interrupt handling");
    processing = false;

    // Arrange
    ITimeOutWatcher *watcherPtr = new SimpleTimeOutWatcher();
    IRoutingManager *routingManager = new BroadcastRoutingManager();
    BGroupForwarding forwarding = BGroupForwarding(term_ptr, *routingManager, *watcherPtr,
                                                   ProgramConfig::getLedFlow());
    const int lastTestRoundTripID = 0xFE;
    forwarding.lastRoundTripId_ = lastTestRoundTripID;
    const int defaultTestRoundTripID = 0x02;
    const int defaultBadRoundTripID = 0xFA;
    const int inBounderyRoundTripID = 0x3F;  // (0xFF-0xFE)+ 0x3F = 0x40 = 64
    const int outBounderyRoundTripID = 0x40;

    // Act
    bool defaultTripIsGreater = forwarding.tripsGreaterThanLast(
            defaultTestRoundTripID);
    bool defaultTripIsLower = forwarding.tripsGreaterThanLast(
            defaultBadRoundTripID);
    bool isEqualTrip = forwarding.tripsGreaterThanLast(lastTestRoundTripID);
    bool isInBoundery = forwarding.tripsGreaterThanLast(inBounderyRoundTripID);
    bool isOutBoundery = forwarding.tripsGreaterThanLast(
            outBounderyRoundTripID);

    // Assert
    // Default Good Test
    UCUNIT_CheckIsEqual(defaultTripIsGreater, true);
    // Default Bad Test
    UCUNIT_CheckIsEqual(defaultTripIsLower, false);
    // Default is Equal Test
    UCUNIT_CheckIsEqual(isEqualTrip, false);
    // Boundery in Test
    UCUNIT_CheckIsEqual(isInBoundery, true);
    // Boundery out Test
    UCUNIT_CheckIsEqual(isOutBoundery, false);

    UCUNIT_TestcaseEnd();
    delete reinterpret_cast<SimpleTimeOutWatcher*>(watcherPtr);
    delete reinterpret_cast<BroadcastRoutingManager*>(routingManager);
}

// This test proof counting from passed nodes during alive transmission
#pragma GCC diagnostic ignored "-Wstack-usage="
void UnitSuiteTests::test004AliveMessageNodeIncrement() {
    UCUNIT_TestcaseBegin("004 Counting passed nodes");
    ITimeOutWatcher *watcherPtr = new SimpleTimeOutWatcher();
    IRoutingManager *routingManager = new BroadcastRoutingManager();
    BGroupForwarding forwarding = BGroupForwarding(term_ptr, *routingManager, *watcherPtr,
                                                   ProgramConfig::getLedFlow());
    ProgramConfig::setCoordinatorStatus(false);
    testRoutingData testData[2] = { { 0, IRoutingManager::Receiver, 2 }, { 1,
            IRoutingManager::Transmitter, 3 } };
    storeTestRoutingDataField(routingManager, testData, 2);
    AliveData testAlive = AliveData(AliveData::incoming, 2,
            IRoutingManager::Transmitter);
    forwarding.setLastRoundTripId(0);
    testAlive.setPassedNodes(2);
    testAlive.setRoundTripID(1);

    UCUNIT_WriteString("Counting allowed:\n");
    forwarding.forward(testAlive);
    UCUNIT_CheckIsEqual(forwarding.outgoingAlive_->getPassedNodes(), 3);

    UCUNIT_WriteString("Counting blocked:\n");
    forwarding.forward(testAlive);
    UCUNIT_CheckIsEqual(forwarding.outgoingAlive_->getPassedNodes(), 3);

    reactOnAliveFromTestRoutingData(forwarding, testData[0]);

    processing = false;
    UCUNIT_TestcaseEnd();
    delete reinterpret_cast<SimpleTimeOutWatcher*>(watcherPtr);
    delete reinterpret_cast<BroadcastRoutingManager*>(routingManager);
}

// This test proof if the alive mapping visualization works as intended
#pragma GCC diagnostic ignored "-Wstack-usage="
void UnitSuiteTests::test006AliveMappingDisplay() {
    UCUNIT_TestcaseBegin("006 Visualization Alive mapping");
    IRoutingManager *routingManager = new BroadcastRoutingManager();
    testRoutingData testData[5] = {
        {0, IRoutingManager::Administrator, IRoutingManager::BGEnd},
        {1, IRoutingManager::Transmitter, 1},
        {2, IRoutingManager::Supervisor, 2},
        {3, IRoutingManager::ListenerF1, 4},
        {4, IRoutingManager::ListenerB1, 3}
    };
    char substring[14];
    storeTestRoutingDataField(routingManager, testData, 5);

    UCUNIT_WriteString(routingManager->showData(0));
    strncpy(substring, Global2::OutBuff + ((26 * 4) + 2), 10);
    UCUNIT_CheckIsEqual(strcmp(substring, "Supervisor"), true);
    processing = false;
    UCUNIT_TestcaseEnd();
    delete reinterpret_cast<BroadcastRoutingManager*>(routingManager);
}


// This tests proofs if the Renesas is implemented an has a version in a specific range
void UnitSuiteTests::test008ProofSPIDriver() {
    UCUNIT_TestcaseBegin("008 Proof access to SPI driver");
    uint32_t version = R_RSPI_GetVersion();
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            "Renesas SPI driver version: %d.%d\r", version>>16, version & 0xFFFF);
    UCUNIT_WriteString(Global2::OutBuff);
    UCUNIT_CheckIsInRange(version, 0x30028, 0x90063);   // 3.40 .. 9.99
    processing = false;
    UCUNIT_TestcaseEnd();
}


#pragma GCC diagnostic ignored "-Wstack-usage="
void UnitSuiteTests::reactOnAliveFromTestRoutingData(BGroupForwarding &forwarding,
        testRoutingData &routingData) {
    AliveData testAlive = AliveData(AliveData::incoming, routingData.ip,
            IRoutingManager::Receiver);
    testAlive.setRoundTripID(1);
    testAlive.setStatus(AliveData::Alive_Request);
    forwarding.reactOnAliveWithHostID(testAlive);
}

void UnitSuiteTests::storeTestRoutingDataField(IRoutingManager *routingManager,
        testRoutingData *routingData, uint8_t fieldLength) {
    for (uint8_t routingDataIndex = 0; routingDataIndex < fieldLength;
            routingDataIndex++) {
        testRoutingData &expectedData = routingData[routingDataIndex];
        routingManager->storeData(IRoutingManager::routingRowEntryIndex,
                expectedData.rowIndex);
        routingManager->storeData(IRoutingManager::GET_MEMBER_ID_AT_INDEX,
                expectedData.role);
        routingManager->storeData(IRoutingManager::GET_IP_AT_INDEX,
                expectedData.ip);
    }
}

// This tests proofs if the alive automation can detect if a network is divided
void UnitSuiteTests::test011ProofNetworkIsCleaved() {
    UCUNIT_TestcaseBegin("011 MST is divided");

    // Arrange
    ModemController::deActivateAllMembers();
    ModemController::activateTestMembersUptoIP(4);
    compArranger.create4PeerNetworkSeparation();
    AssignmentFlow::adjazenzParentPtr_->fillMatrixFromDiscoveryData(false);
    AssignmentFlow::flowStep = AssignmentFlow::proofTree;
    AssignmentFlow::flowSubStep = AssignmentFlow::treeProof;
    AssignmentFlow::setAutomaticMode(true);

    // Act
    AssignmentFlow::doProofTree();
    termFilterPtr_->println(AssignmentFlow::doProofTree());

    // Assert
    UCUNIT_CheckIsEqual(AssignmentFlow::flowStep,
            AssignmentFlow::routeDiscovery);
    UCUNIT_TestcaseEnd();

    AssignmentFlow::flowSubStep = AssignmentFlow::idle;
    AssignmentFlow::flowStep = AssignmentFlow::idle;
    processing = false;
    AssignmentFlow::setAutomaticMode(false);
}

// This test proofs if the alive automation can detect if a network is not divided
void UnitSuiteTests::test012ProofNetworkIsNotCleaved() {
    UCUNIT_TestcaseBegin("012 MST is not split");

    // Arrange
    ModemController::deActivateAllMembers();
    ModemController::activateTestMembersUptoIP(3);
    compArranger.create3PeerLine();
    AssignmentFlow::adjazenzParentPtr_->fillMatrixFromDiscoveryData(false);
    AssignmentFlow::flowStep = AssignmentFlow::proofTree;
    AssignmentFlow::flowSubStep = AssignmentFlow::treeProof;
    AssignmentFlow::setAutomaticMode(true);

    // Act
    AssignmentFlow::doProofTree();
    termFilterPtr_->println(AssignmentFlow::doProofTree());

    // Assert
    UCUNIT_CheckIsEqual(AssignmentFlow::flowStep, AssignmentFlow::sendBCOrder);
    UCUNIT_TestcaseEnd();

    AssignmentFlow::flowSubStep = AssignmentFlow::idle;
    AssignmentFlow::flowStep = AssignmentFlow::idle;
    processing = false;
    AssignmentFlow::setAutomaticMode(false);
}

void UnitSuiteTests::checkRolesWithRoutingDataField(
        IRoutingManager::MemberIpTupel *roles, testRoutingData *routingData,
        uint8_t fieldLength) {
    for (uint8_t roleIndex = 0; roleIndex < fieldLength; roleIndex++) {
        testRoutingData &expectedData = routingData[roleIndex];
        bool isTupelAsExpected = peerTupelIsEqual(roles[roleIndex],
                expectedData.role, expectedData.ip);
        UCUNIT_CheckIsEqual(isTupelAsExpected, true);
    }
}

bool UnitSuiteTests::peerTupelIsEqual(IRoutingManager::MemberIpTupel tupel,
                                      IRoutingManager::eTupelRoles role, uint16_t ip) {
    return ((tupel.meberID == role) && (tupel.ip == ip));
}

//  The following scenario resulted in an error during creation of the MST in previous source code
#pragma GCC diagnostic ignored "-Wstack-usage="
void UnitSuiteTests::test014MSTBigSystem() {
    // Arrange
    const uint8_t participantAmount = 12;
    static bool coordStatus = false;
    static AdjazenzParent *adParent;
    static const AdjazenzMatrixGraph *adMatrix;
    const uint8_t proofMatrix[][participantAmount] = {
            { 00, 58, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 57, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 59, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 00, 44, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 47, 44, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 43, 00, 00, 52, 51, 47, 47 },
            { 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 },
            { 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 }};

    static int rowOffset, colOffset;
    enum testSteps {
        arrange, act, actWait, assert, end,
    };
    static testSteps step = arrange;
    switch (step) {
    case arrange:
        processing = true;
        UCUNIT_TestcaseBegin("014 Network spanning tree test 11 participants");
        ProgramConfig::isCoordinator();
        ProgramConfig::setCoordinatorStatus(true);
        ModemController::activateTestMembersUptoIP(participantAmount);
        compArranger.create11PeerDoubleGroup();
        adParent = AssignmentFlow::getAdjazenzParent();
        adParent->fillMatrixFromDiscoveryData(false);
        adMatrix = &adParent->getAdMatrix();
        DelayHandler::resetTimer();
        rowOffset = 0;
        colOffset = 0;
        step = assert;
        break;
    case assert: {
        bool trigger = true;

        for (int col = 0; col < participantAmount; col++) {
             for (int row = 0; row < participantAmount; row++) {
                if (trigger) {
                    row = rowOffset;
                    col = colOffset;
                    trigger = false;
                }
                const uint16_t wight = adMatrix->getWightByIndex(row, col);
                if (wight > 0) {
                    rowOffset = row+1;
                    colOffset = col;
                    UCUNIT_WriteInt(wight);
                    UCUNIT_CheckIsEqual(proofMatrix[row][col], wight);
                    return;
                }
            }
        }
        step = end;
    }

    break;
    case end:
        step = arrange;
        AssignmentFlow::flowSubStep = AssignmentFlow::idle;
        AssignmentFlow::flowStep = AssignmentFlow::idle;
        ProgramConfig::setCoordinatorStatus(coordStatus);
        UCUNIT_TestcaseEnd();
        processing = false;
        break;
    }
}



#pragma GCC diagnostic ignored "-Wstack-usage="
void UnitSuiteTests::test017IOInputRequestHandling() {
    enum {
        nsduLengthOffset = 2, udpHeaderOffset = 16
    };
    enum respondingTestSteps {
        testSetup, testProcess, testProof, testEnd
    };
    static respondingTestSteps step = testSetup;
    static ModemInterpreter *modemPtr = ProgramConfig::getPlcModemPtr();
    char inputCmd[32];
    const char* expectedInputCmdReply = "::i000:1:0:";
    char generatedInputCmdReply[16];
    static char testMessageContainer[32];
    bool isEqual = false;
    static uint16_t nsduLength;
    static PlcCommand cmd;
    static TermContextFilter::filterBits filter;
    OutputAnalyzer outAnalyzer;

    switch (step) {
    case testSetup:
        UCUNIT_TestcaseBegin("017 IO Input request handling");
        snprintf(inputCmd, sizeof(inputCmd), "%s%c%03d",
                ModemInterpreter::commandPrefix, ModemInterpreter::InputRequest,
                 ModemController::getNetworkIPAddress());
        nsduLength = ModemController::createIPv6udpFrame(
                reinterpret_cast<uint8_t*>(testMessageContainer
                        + nsduLengthOffset),
                reinterpret_cast<uint8_t*>(inputCmd), strlen(inputCmd),
                IRoutingManager::BCGlobalIP, true);
        cmd = PlcCommand(g3Block, channel0, indication, adpLayer, DATA,
                         reinterpret_cast<uint8_t*>(testMessageContainer),
                         (uint8_t) (nsduLength + 5));
        processing = true;
        step = testProcess;
        break;
    case testProcess:
        filter = TermContextFilter::getFilterMask();
        TermContextFilter::setFilterMask(TermContextFilter::filterPLCOutput);
        modemPtr->handelPLCRequests(&cmd);
        step = testProof;
        break;
    case testProof:

        //  hexToString(Global2::OutBuff + udpHeaderOffset, generatedInputCmdReply);
        //  isEqual = (strcmp(generatedInputCmdReply, expectedInputCmdReply) == 0);
        isEqual = outAnalyzer.isGlobalOutputEqualTo(expectedInputCmdReply);
        TermContextFilter::setFilterMask(filter);
        UCUNIT_WriteLine("Expected:");
        UCUNIT_WriteLine(expectedInputCmdReply);
        UCUNIT_WriteLine("Generated:");
        UCUNIT_WriteLine(generatedInputCmdReply);
        UCUNIT_CheckIsEqual(isEqual, true);
        step = testEnd;
        break;
    case testEnd:
        UCUNIT_TestcaseEnd();
        processing = false;
        step = testSetup;
        break;
    }
}


void UnitSuiteTests::test028FilteringOutputs() {
    UCUNIT_TestcaseBegin("028 Check terminal output filter");

    TermContextFilter *pFilter = new TermContextFilter(term_ptr);
    pFilter->setContext(TermContextFilter::filterUnitTest);
    bool testPosFilter = pFilter->isFiltered(TermContextFilter::filterPLCData);
    bool testNegFilter = pFilter->isFiltered(TermContextFilter::filterUnitTest);

    // Assert
    // Default Good Test
    UCUNIT_CheckIsEqual(testPosFilter, true);
    // Default Bad Test
    UCUNIT_CheckIsEqual(testNegFilter, false);

    processing = false;
    UCUNIT_TestcaseEnd();
    delete reinterpret_cast<TermContextFilter*>(pFilter);
}


// Test considering handling of MAC addresses from external i2c device
void UnitSuiteTests::test029MacViaI2c() {
    enum macTestSteps {
        testSetup, testEnd
    };

    switch (testStep) {
    case testSetup:
        UCUNIT_TestcaseBegin("029 Read MAC address from external i2c device");
        i2c_app::i2cMacRead(const_cast<char*>(""));
        processing = true;
        testStep = testEnd;
        break;
    case testEnd: {
        // check if MAC address is read.
        bool isAvailable = i2c_app::isMacAdrAvailable();
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                "MAC device is%s available", (isAvailable) ? "" : " not");
        UCUNIT_WriteLine(Global2::OutBuff);
        if (isAvailable) {
            uint8_t macRead[8];         // std::array<uint8_t, 8>
            i2c_app::getI2MacAdr(macRead);

            UCUNIT_WriteLine("Read MAC address:");
            for (unsigned int i = 0; i < sizeof(macRead); i++) {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%02x",
                        macRead[i]);
                UCUNIT_WriteString(Global2::OutBuff);
                if (i != sizeof(macRead) - 1) {
                    UCUNIT_Write(',');
                    UCUNIT_Write(' ');
                }
            }
            UCUNIT_WriteLine("");

            // Compare first 6 bytes of address with a template
            // Hint:
            // The first 6 bytes of the modules used are the same.
            // The following 2 bytes are crucial to distinguish between the modules
            uint8_t macReference[6] = { 0, 0x4, 0xa3, 0x0b, 0x01, 0x1b };
            UCUNIT_WriteString("Has device the leading address (first 6 bytes) ");
            for (unsigned int i = 0; i < sizeof(macReference); i++) {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%02x", macReference[i]);
                UCUNIT_WriteString(Global2::OutBuff);
                if (i != sizeof(macReference) - 1) {
                    UCUNIT_Write(',');
                    UCUNIT_Write(' ');
                }
            }
            UCUNIT_WriteLine("?");
            int cmp = memcmp(macRead, macReference, sizeof(macReference));
            bool isEqual = cmp == 0;
            UCUNIT_CheckIsEqual(isEqual, true);
        } else {  // not available
            UCUNIT_CheckIsEqual(isAvailable, false);
        }
        UCUNIT_TestcaseEnd();
        testStep = testSetup;
        processing = false;
            break;
        }

        default:
            testStep = testSetup;
        }
    }

// Test considering initialization of hardware
void UnitSuiteTests::test031InitHardware() {
    UCUNIT_TestcaseBegin("031 Proof initialization of hardware driver");
    UCUNIT_WriteString(PCBPortSelector::versionToString(Global2::OutBuff, Global2::outBuffMaxLength));
    UCUNIT_Write(SerialStandardProfile::RETURN);

    IoPin *CPX3Boot = PCBPortSelector::getPin(PCBPortSelector::CPX3Boot);
    bool resetPinValid = PCBPortSelector::isValidPin(CPX3Boot);
    UCUNIT_CheckIsEqual(true, resetPinValid);

    IoPin *led1 = PCBPortSelector::getPin(PCBPortSelector::LED, 1);
    bool ledValid = PCBPortSelector::isValidPin(led1);
    UCUNIT_CheckIsEqual(true, ledValid);

    IoPinGroup* inputs = PCBPortSelector::getGroup(PCBPortSelector::ModuleInGroup);
    bool inputGroupValid = PCBPortSelector::isValidGroup(inputs);
    UCUNIT_CheckIsEqual(true, inputGroupValid);

    IoPinGroup* outputs = PCBPortSelector::getGroup(PCBPortSelector::ModuleOutGroup);
    bool outputGroupValid = PCBPortSelector::isValidGroup(outputs);
    UCUNIT_CheckIsEqual(true, outputGroupValid);
    UCUNIT_TestcaseEnd();
}

}  // namespace AME_SRC
