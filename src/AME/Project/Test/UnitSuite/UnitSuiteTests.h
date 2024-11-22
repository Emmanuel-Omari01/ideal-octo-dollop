/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */
#ifndef SRC_AME_PROJECT_TEST_UNITSUITE_UNITSUITETESTS_H_
#define SRC_AME_PROJECT_TEST_UNITSUITE_UNITSUITETESTS_H_

#include "PeerCompositionArranger.h"
#include "UpdateModuleTests.h"

#include <cstdint>

#include "../../../Project/Terminal/SerialDrivers/SerialDriver.h"
#include "../../../Project/Terminal/TermContextFilter.h"
#include "../../../Project/PlcCommunication/AliveHandling/MessageForwarding/ForwardingFlow.h"
#include "../../../Project/PlcCommunication/AliveHandling/MessageForwarding/BGroupForwarding.h"

namespace AME_SRC {

class UnitSuiteTests {
 public:
    struct testRoutingData {
        uint8_t rowIndex;
        IRoutingManager::eTupelRoles role;
        uint16_t ip;
    };

    enum eAliveTestMode {
        start, stop, proceed,
    };

    explicit UnitSuiteTests(SerialDriver *term);
    void runTests();
    virtual ~UnitSuiteTests();
    bool isTested() {
        return testsFinished;
    }
    void setTestEnabled(bool set) {
        this->testsEnabled = set;
        if (set) {
            testStep = 0;   // Reset Step => Run test again
            testsFinished = false;
            termFilterPtr_->setFilterMask(TermContextFilter::filterUnitTest);
        }
    }
    bool getTestEnabled() {
        return testsEnabled;
    }
    void setTestParameter(uint8_t para) {
        this->testParameter = para;
    }
    uint8_t getTestParameter() {
        return testParameter;
    }

    static void setProcessing(bool status) {
        processing = status;
    }
    static void setLastUnitTest(bool set);
    static bool getLastUnitTest();

 private:
    PeerCompositionArranger compArranger;
    SerialDriver *term_ptr;
    static TermContextFilter *termFilterPtr_;
    static bool processing;
    bool testsFinished;
    bool testsEnabled;
    uint8_t testStep;
    uint8_t testParameter;
    UpdateModuleTests *pUpdateModuleTests;

    uint8_t* tsrd_prepare_stream(uint8_t *stream, uint32_t streamLen, uint8_t ip, uint8_t count);
    uint8_t* tsrd_add_stream(uint8_t *stream, uint16_t dest_ip,
            uint8_t linkQuality);
    void fakeMemberIps(uint16_t *ips, uint8_t length, bool start);
    void fakeGivenAmountOfMemberIps(uint8_t fakeAmount, bool &hasFinished);
    void test001RoleLineAssignment();
    void test003RouteIdIsChecked();
    void test004AliveMessageNodeIncrement();
    void test006AliveMappingDisplay();
    void test008ProofSPIDriver();
    void test011ProofNetworkIsCleaved();
    void test012ProofNetworkIsNotCleaved();
    void test014MSTBigSystem();
    void test017IOInputRequestHandling();
    void test028FilteringOutputs();
    void test029MacViaI2c();
    void test031InitHardware();

    void reactOnAliveFromTestRoutingData(BGroupForwarding &forwarding,
            testRoutingData &routingData);
    void storeTestRoutingDataField(IRoutingManager* routingManager,
            testRoutingData *routingData, uint8_t fieldLength);
    void checkRolesWithRoutingDataField(IRoutingManager::MemberIpTupel*,
            testRoutingData *routingData, uint8_t fieldLength);
    bool peerTupelIsEqual(IRoutingManager::MemberIpTupel tupel,
                          IRoutingManager::eTupelRoles, uint16_t ip);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TEST_UNITSUITE_UNITSUITETESTS_H_
