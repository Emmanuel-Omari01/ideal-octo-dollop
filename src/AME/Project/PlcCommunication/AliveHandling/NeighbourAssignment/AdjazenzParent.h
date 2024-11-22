/*
 * AdjazenzParent.h
 *
 *  Created on: 20.06.2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ADJAZENZPARENT_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ADJAZENZPARENT_H_

#include "AdjazenzMatrixGraph.h"
#include "MinSpanTreeCalculator.h"

#include <cstdint>

#include "../../../HelpStructures/List.h"
#include "../../../Terminal/SerialDrivers/SerialDriver.h"
#include "../../../Terminal/TermContextFilter.h"
#include "../../../HelpStructures/BitMask.h"

namespace AME_SRC {

class AdjazenzParent {
 public:
    explicit AdjazenzParent(SerialDriver *notifyRef);
    virtual ~AdjazenzParent();

    typedef struct {
        uint16_t source;
        uint16_t destination;
    } tuple;
    // static List<tuple> *pRouteList;  // Attention: dynamic access does not work yet. See Issue #107

    void fillMatrixFromDiscoveryData(bool minSpannBaum);
    void shortenRouteList();
    void showMatrixOrigin(char *output, int outLen);
    void showGraphLine(char *output, int outLen, bool *done);
    void showRouteLine(char *output, int outLen, bool *done);
    uint16_t getRouteListEntries(bool shortEndTree) {
        return (shortEndTree) ?
                (uint16_t) (lastRouteIndex + 1) : routeListEntries;
    }
    tuple* getRoute() {
        return aRouteList;
    }
    bool setRoute(const char *input);
    uint16_t getMaxSource();
    void showMatrixOnTerminal();
    void printAndWaitUntilDone();
    bool getRouteFailed() {
        return routeFailed;
    }

    static const AdjazenzMatrixGraph& getAdMatrix() {
        return adMatrix;
    }

 private:
    static TermContextFilter *termFilterPtr_;
    void calcTreeRoute();
    bool calcTRHelper(uint16_t next, uint16_t back, uint16_t nTiefe);
    bool addTreeRoute(uint16_t source, uint16_t destination);
    bool isNewTreeRoutePossible();
    void retrieveLastRouteIndex();

    static AdjazenzMatrixGraph adMatrix;
    static IGraph *myGraphPtr;
    static MinSpanTreeCalculator minSpanCalc;
    static IMinSpanTreeCalculator *treeCalc;

#define kRouteListSize  100
    static tuple aRouteList[kRouteListSize];
    static uint16_t routeListEntries;
    static uint16_t routeListDisplayIndex;
    static uint16_t lastRouteIndex;
    static uint16_t verticesAmount;
    static BitMask<uint32_t> calcMask;
    bool routeFailed;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ADJAZENZPARENT_H_
