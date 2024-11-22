/*
 * TermContextFilter.cpp
 *
 *  Created on: Jun 11, 2024
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 *
 *  Short: Class for creating a way to filter text output on the user terminal
 */

#include "TermContextFilter.h"

#include <cstdio>

#include "../SystemBase/Global2.h"

namespace AME_SRC {

uint32_t TermContextFilter::filterMask = filterL2 |
        filterPLCData |
        filterAliveRxInfo |
        filterAliveTxInfo |
        filterPLCText |
        filterJoinTabs |
        filterInitLoadBar |
        filterRoutingInfo |
        filterAliveJumpInfo |
        filterIntegrationTest;

TermContextFilter::TermContextFilter(SerialDriver *ptrDrive):
        actualContext((filterBits) filterNone) {
    if (ptrDrive) {
        termDrive = ptrDrive;
    }
}

TermContextFilter::~TermContextFilter() {
    // TODO(AME): Auto-generated destructor stub
}

void TermContextFilter::setFilterMask(filterBits setMask) {
    filterMask = setMask;
}

void TermContextFilter::setFilterStandard() {
    filterMask = filterVerboseStandard;
}


TermContextFilter::filterBits TermContextFilter::getFilterMask() {
    return (filterBits) filterMask;
}

void TermContextFilter::setContext(filterBits setFilter) {
    actualContext = setFilter;
}


void TermContextFilter::releaseContext() {
    actualContext = filterNone;
}


bool TermContextFilter::isNotFiltered(filterBits context) {
    return (context  & filterMask) != 0;
}

bool TermContextFilter::isNotFiltered() {
    return (actualContext & filterMask) != 0;
}

bool TermContextFilter::isFiltered(filterBits context) {
    return !isNotFiltered(context);
}


void TermContextFilter::putchar(char numChar) {
    if (isNotFiltered()) {
        termDrive->putchar(numChar);
    }
}

void TermContextFilter::print(const char *str) {
    if (isNotFiltered()) {
        termDrive->print(str);
    }
}

void TermContextFilter::print(TX::textNumber nr) {
    if (isNotFiltered()) {
        termDrive->print(TX::getText(nr));
    }
}

void TermContextFilter::println(TX::textNumber nr) {
    if (isNotFiltered()) {
        termDrive->println(TX::getText(nr));
    }
}

void TermContextFilter::println(const char *str) {
    if (isNotFiltered()) {
        termDrive->println(str);
    }
}


void TermContextFilter::print(const char *str, filterBits context) {
    uint32_t lastContext = actualContext;
    setContext((filterBits) (context | actualContext));
    print(str);
    setContext((filterBits) lastContext);
}

void TermContextFilter::println(const char *str, filterBits context) {
    uint32_t lastContext = actualContext;
    setContext((filterBits) (context | actualContext));
    println(str);
    setContext((filterBits) lastContext);
}


void TermContextFilter::print(TX::textNumber nr, filterBits context) {
    uint32_t lastContext = actualContext;
    setContext((filterBits) (context | actualContext));
    print(TX::getText(nr));
    setContext((filterBits) lastContext);
}

void TermContextFilter::println(TX::textNumber nr, filterBits context) {
    uint32_t lastContext = actualContext;
    setContext((filterBits) (context | actualContext));
    println(TX::getText(nr));
    setContext((filterBits) lastContext);
}

void TermContextFilter::printIntln(int n) {
    if (isNotFiltered()) {
        termDrive->printIntLN(n);
    }
}

void TermContextFilter::printP(const char *fmt, ...) {
    int n;
    va_list ap;
    va_start(ap, fmt);
    n = vsnprintf(Global2::OutBuff, Global2::outBuffMaxLength, fmt, ap);
    va_end(ap);
    if (n < 0) {
        return;
    }
    print(Global2::OutBuff);
}


void TermContextFilter::printPln(const char *fmt, ...) {
    int n;
    va_list ap;
    va_start(ap, fmt);
    n = vsnprintf(Global2::OutBuff, Global2::outBuffMaxLength, fmt, ap);
    va_end(ap);
    if (n < 0) {
        return;
    }
    println(Global2::OutBuff);
}


}  // namespace AME_SRC
