/*
 * IntegrationsTests.h
 *
 *  Created on: Apr 29, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TEST_INTEGRATION_INTEGRATIONSTESTS_H_
#define SRC_AME_PROJECT_TEST_INTEGRATION_INTEGRATIONSTESTS_H_

#include <cstdint>

#include "../../../Project/Terminal/SerialDrivers/SerialDriver.h"
#include "../../../Project/PlcCommunication/AliveHandling/MessageForwarding/ForwardingFlow.h"

namespace AME_SRC {

class IntegrationsTests {
 public:
    enum eTestcases {
        CaseTest030AliveRoundTrips = 1,
    };
    enum eAliveTestMode {
        start, stop, proceed,
    };
    enum eConst {
       peerAlarmLimit = 3  // number of round trips where peer is not participant of the loop
    };

    struct testParameters {
        uint32_t testingTime;       // [s]
        uint16_t numberOfTest;      // select test
        uint16_t numberOfPeers;
    };

    // Enumeration for request to testParameter
    enum eParameterIndex {
        eTestingTime, eNumberOfTest, eNumberOfPeers
    };

    IntegrationsTests(SerialDriver *term, ForwardingFlow &forwardingFlow);

    virtual ~IntegrationsTests();

    void runTests();

    bool isTested() {
        return testsFinished;
    }
    bool isAliveTestRunning() {
        return data.numberOfTest == CaseTest030AliveRoundTrips && processing;
    }
    void testAliveStop();
    void test030AliveRoundTrips(eAliveTestMode mode);
    bool scanPeerList(uint32_t actualRuntime);
    bool checkAliveFail(bool withOutput);

    void setTestEnabled(bool set) {
        this->testsEnabled = set;
        if (set) {
            testStep = 0;   // Reset Step => Run test again
            testsFinished = false;
            termFilterPtr_->setFilterMask(TermContextFilter::filterIntegrationTest);
        }
    }
    bool getTestEnabled() {
        return testsEnabled;
    }

    void setTestParameter(eParameterIndex select, uint32_t value ) {
        switch (select) {
            case eTestingTime:
                data.testingTime = value;
                break;
            case eNumberOfTest:
                data.numberOfTest = (uint16_t) value;
                break;
            case eNumberOfPeers:
                data.numberOfPeers = (uint16_t) value;
                break;
        }
    }
    uint32_t getTestParameter(eParameterIndex select) {
        switch (select) {
            case eTestingTime:
                return data.testingTime;
            case eNumberOfTest:
                return (uint32_t) data.numberOfTest;
            case eNumberOfPeers:
                return (uint32_t) data.numberOfPeers;
        }
        return 0;
    }

 private:
    struct sPeerLostInfo {
        uint8_t actuallyLost:1;   // peer that are actually no participant of the alive round trips
        uint8_t lostOccurance:1;  // peer which were partial not participant of the alive round trips
    };

    SerialDriver* term_ptr;
    static TermContextFilter *termFilterPtr_;
    ForwardingFlow& _forwardingFlowRef;
    sPeerLostInfo *peersWithTimeout;
    bool processing;
    bool testsFinished;
    bool testsEnabled;
    testParameters data;
    uint8_t testStep;
};

}  // namespace AME_SRC

extern "C" void UCUnit_setLastTest(bool set);

#endif  // SRC_AME_PROJECT_TEST_INTEGRATION_INTEGRATIONSTESTS_H_
