/*
 * Integration.cpp
 *
 *  Created on: Apr 29, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "IntegrationsTests.h"

#include <cstdio>
#include <cstring>

#include "../../PlcCommunication/AliveHandling/NeighbourAssignment/AssignmentFlow.h"
#include "../../PlcCommunication/ModemController.h"
#include "../../Terminal/SerialProfiles/SerialStandardProfile.h"
extern "C" {
#include "../../../../OpenSource/uCUnit/uCUnit-v1.0.h"
}

namespace AME_SRC {

TermContextFilter *IntegrationsTests::termFilterPtr_;

IntegrationsTests::IntegrationsTests(AME_SRC::SerialDriver *term,
        ForwardingFlow &forwardingFlow) :
        _forwardingFlowRef(forwardingFlow),
        processing(false),
        testsFinished(false), testsEnabled(false), testStep(0) {
    this->term_ptr = term;
    termFilterPtr_ = new TermContextFilter(term);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    data.numberOfTest = 0;
    data.numberOfPeers = 1;
    data.testingTime = 10;
    peersWithTimeout = nullptr;
}

IntegrationsTests::~IntegrationsTests() = default;

// Entry to run tests of testsuite.
// runTests is called with an interval of 1s as soon as modem is initialized
void IntegrationsTests::runTests() {
    if (!testsEnabled) {
        return;
    }

    switch (data.numberOfTest) {
       case CaseTest030AliveRoundTrips:
           test030AliveRoundTrips(proceed);
           break;
       default:
           termFilterPtr_->print(TX::dTxtIntegration001);
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
}


void IntegrationsTests::test030AliveRoundTrips(eAliveTestMode mode) {
    enum internalSteps {
        idle, setup, pre_Check, check_conditions, run,
        end_sub1, end_sub2, end_sub3, end_sub4, end_sub5,
    };
    const uint32_t resolution1s = 1;        // 1s
    static uint8_t step = setup;
    static bool restart = false;
    static uint32_t runTimeSecs = 0;        // [1s]: 0... ~136 years
    char cResult[10];
    bool bResult;

    switch (mode) {
        case stop:
            if (step != idle && step != end_sub1) {
                step = end_sub1;
            }
            break;
        case start:
            if (step != idle && step != setup) {
                step = end_sub1;
                restart = true;
            } else {
                step = setup;
            }
            break;
        default:
        case proceed:
            break;
    }

    runTimeSecs++;

    switch (step) {
        case setup:
            UCUnit_setLastTest(true);
            UCUNIT_TestcaseBegin("030 Alive round trips start");
            // create a bit field on heap
            if (nullptr == peersWithTimeout) {
                int max = ModemController::getNetworkMemberMax();
                peersWithTimeout = new sPeerLostInfo[max+1];
                if (peersWithTimeout) {
                    memset(peersWithTimeout, 0, max * sizeof(sPeerLostInfo));
                }
            }

            // stop automatic
            AssignmentFlow::setFlowSteps(AssignmentFlow::idle, 0, const_cast<char *>("Integration test"));
            AssignmentFlow::clearAllFlowTimeouts();

            // Test is running for xx seconds
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::dTxtIntegration011), data.testingTime);
            UCUNIT_WriteLine(Global2::OutBuff);

            processing = true;
            runTimeSecs = 0;
            // set minimum number of peers,
            // which have to be within the alive round-trips
            AssignmentFlow::setData("numMinPeers", 1, data.numberOfPeers);

            // start automatic
            AssignmentFlow::setAutomaticMode(true);
            AssignmentFlow::getData("status", cResult, sizeof(cResult), "%d", 1, "remoteDiscovered");
            bResult = static_cast<bool>(atoi(cResult));
            if (bResult) {
                AssignmentFlow::getData("status", cResult, sizeof(cResult), "%d", 1, "mstBuilded");
                bResult = static_cast<bool>(atoi(cResult));
                if (bResult) {
                    AssignmentFlow::getData("status", cResult, sizeof(cResult), "%d", 1, "routeAssigned");
                    bResult = static_cast<bool>(atoi(cResult));
                    if (bResult) {
                      ForwardingFlow::callStartAliveRoundTrips(" -1", true, &bResult);
                    } else {
                        AssignmentFlow::runFunction("assignRoute");
                    }
                } else {
                    AssignmentFlow::runFunction("buildTree");
                }
            } else {
                AssignmentFlow::runFunction("all");
            }

            step = pre_Check;
            break;

        case pre_Check:
        case check_conditions: {
            AssignmentFlow::getData("status", cResult, sizeof(cResult), "%d", 1, "dwellTimeCompleted");
            bool bDwell = static_cast<bool>(atoi(cResult));
            AssignmentFlow::getData("status", cResult, sizeof(cResult), "%d", 1, "isIdle");
            bResult = static_cast<bool>(atoi(cResult));
            if (bDwell & bResult) {
                if (step == check_conditions) {
                    if (checkAliveFail(false)) {
                        step = end_sub1;
                    } else {
                        step = run;
                    }
                } else {
                    step = check_conditions;
                }
            } else {
                step = pre_Check;
            }

            if (runTimeSecs >= data.testingTime) {
                UCUNIT_Write(SerialStandardProfile::RETURN);
                step = end_sub1;
            }
            break;
        }
        case run:
            if (!scanPeerList(runTimeSecs)) {
                if (runTimeSecs % resolution1s == 0) {
                    UCUNIT_Write('.');
                }
            }
            if (runTimeSecs >= data.testingTime) {
                UCUNIT_Write(SerialStandardProfile::RETURN);
                step = end_sub1;
            }
            break;
        case end_sub1: {
            UCUNIT_WriteString("\r");
            checkAliveFail(true);
            step = end_sub2;
            break; }

        case end_sub2: {
            // Number of tested round trips
            unsigned int trips = _forwardingFlowRef.getRoundTripCount();
            snprintf(cResult, sizeof(cResult), "%d", trips);
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::dTxtIntegration002), cResult);
            UCUNIT_WriteLine(Global2::OutBuff);

            // Number of round trips where repeat timeouts were fired
            trips = _forwardingFlowRef.getRoundTripRepeatCallOuts();
            snprintf(cResult, sizeof(cResult), "%d", trips);
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                    TX::getText(TX::dTxtIntegration012), cResult);
            UCUNIT_WriteLine(Global2::OutBuff);
            UCUNIT_CheckIsEqual(0, trips);
            step = end_sub3;
          break; }

        case end_sub3:
            if (peersWithTimeout) {
                // "Peer who temporarily did not participate in the alive round trips:"
                UCUNIT_WriteLine(TX::getText(TX::dTxtIntegration013));
                int max = ModemController::getNetworkMemberMax();
                bResult = false;
                sPeerLostInfo *p = peersWithTimeout+1;
                for (int i=1; i <= max; i++) {
                    if (p->lostOccurance) {
                        if (bResult) {
                            UCUNIT_Write(',');
                            UCUNIT_Write(' ');
                        }
                        bResult = true;
                        UCUNIT_WriteInt(i);
                    }
                    p++;
                }
                if (!bResult) {
                    UCUNIT_WriteString(TX::getText(TX::txtNone));
                }
                UCUNIT_Write(SerialStandardProfile::RETURN);
            }
            step = end_sub4;
            break;
        case end_sub4:
            if (peersWithTimeout) {
                // "Peers who are not participants of the alive round trips anymore:"
                UCUNIT_WriteLine(TX::getText(TX::dTxtIntegration007));
                int max = ModemController::getNetworkMemberMax();
                bResult = false;
                sPeerLostInfo *p = peersWithTimeout+1;
                for (int i=1; i <= max; i++) {
                    if (p->actuallyLost) {
                        if (bResult) {
                            UCUNIT_Write(',');
                            UCUNIT_Write(' ');
                        }
                        bResult = true;
                        UCUNIT_WriteInt(i);
                    }
                    p++;
                }
                if (!bResult) {
                    UCUNIT_WriteString(TX::getText(TX::txtNone));
                }
                UCUNIT_Write(SerialStandardProfile::RETURN);
            }
            step = end_sub5;
            break;

        case end_sub5:
            UCUNIT_TestcaseEnd();
            if (restart) {
                restart = false;
                step = setup;
            } else {
                processing = false;
                step = idle;
              ForwardingFlow::callStopAliveRoundTrips();
                AssignmentFlow::setFlowSteps(AssignmentFlow::idle, 0, const_cast<char *>("Integration test"));
                AssignmentFlow::clearAllFlowTimeouts();
                delete [] peersWithTimeout;
                peersWithTimeout = nullptr;
            }
            break;
        default:
            break;
    }
}


#pragma GCC diagnostic ignored "-Wstack-usage="
bool IntegrationsTests::scanPeerList(uint32_t actualRuntime) {
    if (nullptr == peersWithTimeout) {
        return false;
    }
    sPeerLostInfo *p = peersWithTimeout +1;
    bool printed = false;
    char out[80];
    int max = ModemController::getNetworkMemberMax();
    enum TX::textNumber txtNo;
    for (int i=1; i <= max; i++) {
        bool status = false;
        // retrieve if there is an alarm for peer i
        NetworkMember* netMember = ModemController::getNetMemberPtrByIndex((uint16_t) i);
        if (netMember && netMember->getAliveMember()) {
            status = netMember->getAliveTimeout() >= peerAlarmLimit;
        }
        bool last = p->actuallyLost;
        if (status != last) {
            if (status & !last) {  // coming error
                p->lostOccurance = true;
                txtNo = TX::dTxtIntegration009;
            } else {  // leaving error
                txtNo = TX::dTxtIntegration010;
            }
            snprintf(out, sizeof(out), TX::getText(txtNo), i, actualRuntime);
            UCUNIT_WriteLine(out);
            printed = true;
            p->actuallyLost = status;
        }
        p++;
    }
    return printed;
}


bool IntegrationsTests::checkAliveFail(bool withOutput) {
    bool failed = false;
    char cResult[10];
    bool bResult;

    AssignmentFlow::getData("numMinPeers", cResult, sizeof(cResult), "%03d", 0);
    unsigned int numMinPeers = atoi(cResult);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtIntegration003), cResult);
    if (withOutput) {
        UCUNIT_WriteLine(Global2::OutBuff);
    }

    AssignmentFlow::getData("numConnectedPeers", cResult, sizeof(cResult), "%03d", 0);
    unsigned int numConnectedPeers = atoi(cResult);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtIntegration004), cResult);
    if (withOutput) {
        UCUNIT_WriteLine(Global2::OutBuff);
        UCUNIT_CheckIsEqual(true, numConnectedPeers >= numMinPeers);
    } else {
        if (numConnectedPeers < numMinPeers) {
            failed = true;
        }
    }

    AssignmentFlow::getData("status", cResult, sizeof(cResult), "%d", 1, "mstBuilded");
    bResult = static_cast<bool>(atoi(cResult));
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtIntegration005),
            (bResult)? TX::getText(TX::txtYes): TX::getText(TX::txtNo));
    if (withOutput) {
        UCUNIT_WriteLine(Global2::OutBuff);
        UCUNIT_CheckIsEqual(true, bResult);
    } else {
        if (!bResult) {
            failed = true;
        }
    }

    AssignmentFlow::getData("status", cResult, sizeof(cResult), "%d", 1, "routeAssigned");
    bResult = static_cast<bool>(atoi(cResult));
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtIntegration006),
            (bResult)? TX::getText(TX::txtYes): TX::getText(TX::txtNo));
    if (withOutput) {
        UCUNIT_WriteLine(Global2::OutBuff);
        UCUNIT_CheckIsEqual(true, bResult);
    } else {
        if (!bResult) {
            failed = true;
        }
    }

    return failed;
}


void IntegrationsTests::testAliveStop() {
  test030AliveRoundTrips(stop);
}

}  // namespace AME_SRC
