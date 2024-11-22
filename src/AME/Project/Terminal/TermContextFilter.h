/*
 * TermContextFilter.h
 *
 *  Created on: Jun 11, 2024
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_TERMCONTEXTFILTER_H_
#define SRC_AME_PROJECT_TERMINAL_TERMCONTEXTFILTER_H_

#include <cstdint>
#include <cstdarg>

#include "../Terminal/SerialDrivers/SerialDriver.h"
#include "../SystemLogic/I18N.h"

namespace AME_SRC {

class TermContextFilter {
 public:
    explicit TermContextFilter(SerialDriver *ptrDrive);
    virtual ~TermContextFilter();

    enum filterBits {  // bit masks to filter terminal outputs
        filterNone = 0,
        filterL1 = 1,                     // filterL1 + filterL2 as pre-selection for terminal outputs (level 1)
        filterL2 = 2,                     //
        filterOutputL1 = 4,               //
        filterPLCInput = 8,                // display of PLC Input data
        filterPLCText = 0x10,             // display text of receiving PLC telegram
        filterJoinTabs = 0x20,            // refresh of the table concerning the force join process
        filterForceJoinS = 0x40,          // display of force join steps
    //  filterInitTokens = 0x40,          // display of message tokens
        filterDiscoveryInfo = 0x80,
        filterInitLoadBar = 0x100,        // display of a loading bar
        filterRoutingInfo = 0x200,        // display of routing information: spanning tree, routing path
        filterAliveRxInfo = 0x400,        // display of an receiving alive telegram in the main process
        filterAliveTxInfo = 0x800,        // display of an alive telegram in the sending process
        filterForceJoinInfo = 0x1000,     // display of a telegram concerning force join
        filterAliveJumpInfo = 0x2000,     // display of details of alive forwarding
        filterDevelope = 0x8000,
        filterUnitTest = 0x10000,         // specific output during unit tests
        filterIntegrationTest = 0x20000,  // specific output during integration tests
        filterPLCOutput = 0x40000,        // display of PLC Output data
        filterPLCData = filterPLCInput | filterPLCOutput,
        filterVerboseStandard = filterL2 | filterPLCData | filterAliveRxInfo | filterAliveTxInfo |
                                filterPLCText | filterJoinTabs | filterInitLoadBar | filterRoutingInfo |
                                filterAliveJumpInfo,
    };

    /***
    uint32_t PlcTerminalCommandConnector::verboseMask = verboseL2|
                                                        verbosePLCData|
                                                        verboseAliveRxInfo|
                                                        verboseAliveTxInfo|
                                                        verbosePLCText|
                                                        verboseJoinTabs|
                                                        verboseInitLoadBar|
                                                        verboseRoutingInfo|
                                                        verboseAliveJumpInfo|
                                                        verboseIntegrationTest;
    ***/

    static void setFilterMask(filterBits setMask);
    static void setFilterStandard();
    static filterBits getFilterMask();
    void releaseContext();
    void setContext(filterBits setFilter);

    void putchar(char numChar);
    void print(const char *str);
    void print(TX::textNumber nr);
    void print(const char *str, filterBits context);
    void print(TX::textNumber nr, filterBits context);
    void println(const char *str);
    void println(TX::textNumber nr);
    void println(const char *str, filterBits context);
    void println(TX::textNumber nr, filterBits context);
    void printP(const char *fmt, ...);
    void printIntln(int n);
    void printPln(const char *fmt, ...);
    static bool isFiltered(filterBits context);
    static bool isNotFiltered(filterBits context);
    bool isNotFiltered();
    int  txdFree() {
        return termDrive->txdFree();
    }
    bool isTransmissionComplete() {
        return termDrive->isTransmissionComplete();
    }
    void popTxD() {
        termDrive->printFast("");
    }
    char *get_buffer() {
        return termDrive->get_buffer();
    }

 private:
    SerialDriver *termDrive;              // reference to the terminal
    static uint32_t filterMask;           // elements of filterBits
    uint32_t actualContext;               // elements of filterBits
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_TERMCONTEXTFILTER_H_
