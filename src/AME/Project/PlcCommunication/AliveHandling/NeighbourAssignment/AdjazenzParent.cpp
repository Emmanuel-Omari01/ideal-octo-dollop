/*
 * AdjazenzParent.cpp
 *
 *  Created on: 20.06.2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "AdjazenzParent.h"

#include "AdjazenzMatrixGraph.h"
#include "IDiscoveryManager.h"
#include "../../../SystemBase/ProgramConfig.h"
#include "SimpleRouteGenerator.h"
#include "../../../HelpStructures/CharOperations.h"

namespace AME_SRC {

AdjazenzMatrixGraph AdjazenzParent::adMatrix;
IGraph *AdjazenzParent::myGraphPtr;
MinSpanTreeCalculator AdjazenzParent::minSpanCalc;
SimpleRouteGenerator simpleRoutGen;
IMinSpanTreeCalculator *AdjazenzParent::treeCalc;
// List<AdjazenzParent::tuple> *AdjazenzParent::pRouteList = NULL;
AdjazenzParent::tuple AdjazenzParent::aRouteList[kRouteListSize];
TermContextFilter *AdjazenzParent::termFilterPtr_ = NULL;
uint16_t AdjazenzParent::routeListEntries = 0;
uint16_t AdjazenzParent::routeListDisplayIndex = 0;
uint16_t AdjazenzParent::lastRouteIndex = 0;
uint16_t AdjazenzParent::verticesAmount = 0;
BitMask<uint32_t> AdjazenzParent::calcMask;

AdjazenzParent::AdjazenzParent(SerialDriver *notifyRef) : routeFailed(true) {
    termFilterPtr_ = new TermContextFilter(notifyRef);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
}

AdjazenzParent::~AdjazenzParent() = default;

void AdjazenzParent::fillMatrixFromDiscoveryData(bool minSpannBaum) {
    ModemInterpreter *plcModemPtr = ProgramConfig::getPlcModemPtr();
    IDiscoveryManager *discManagerPtr = plcModemPtr->getDiscManagerPtr();
    NetworkMember *pn;
    char requestBuf[20];

    myGraphPtr = &adMatrix;
    myGraphPtr->clearMatrix();
    treeCalc = &minSpanCalc;     // simpleRoutGen;
    for (uint32_t i = 0; i < AdjazenzMatrixGraph::maxMatrixIdx; i++) {
        for (uint32_t j = i + 1; j < AdjazenzMatrixGraph::maxMatrixIdx; j++) {
            if (i) {
                pn = ModemController::getNetMemberPtrByIndex((uint16_t) i);
                if ((pn == NULL)
                        || (pn && pn->getLink() != NetworkMember::connected)) {
                    continue;
                }
            }
            pn = ModemController::getNetMemberPtrByIndex((uint16_t) j);
            if ((pn == NULL)
                    || (pn && pn->getLink() != NetworkMember::connected)) {
                continue;
            }

            snprintf(requestBuf, sizeof(requestBuf), "LQI-Data %3u %3u", (uint16_t) i, (uint16_t) j);
            uint16_t linkQuality1 = discManagerPtr->getData(requestBuf);
            snprintf(requestBuf, sizeof(requestBuf), "LQI-Data %3u %3u", (uint16_t) j, (uint16_t) i);
            uint16_t linkQuality2 = discManagerPtr->getData(requestBuf);
            uint16_t lqMid;
            if (linkQuality1 == 0) {
                lqMid = linkQuality2;
            } else if (linkQuality2 == 0) {
                lqMid = linkQuality1;
            } else {
                lqMid = (uint16_t) (linkQuality1 + linkQuality2) / 2;
            }

            if (lqMid) {
                myGraphPtr->addEdge(i, j, lqMid);
                myGraphPtr->addEdge(j, i, lqMid);
            }
        }
    }

    if (!minSpannBaum) {
        verticesAmount = (uint16_t) myGraphPtr->getVertices();
        showMatrixOnTerminal();
        treeCalc->loadData(myGraphPtr);
        myGraphPtr = treeCalc->compute();
        showMatrixOnTerminal();
        calcTreeRoute();

        retrieveLastRouteIndex();
        routeListDisplayIndex = 0;      // pRouteList->resetIndex();
    }
}

void AdjazenzParent::showMatrixOrigin(char *output, int outLen) {
    snprintf(output, outLen, TX::getText(TX::OriginOfMatrix),
            adMatrix.getMatrixOrigin());
}

void AdjazenzParent::showGraphLine(char *output, int outLen, bool *done) {
    if (myGraphPtr->isDisplayed()) {
        *output = 0;
        *done = true;
    } else {
        snprintf(output, outLen, "%s", myGraphPtr->toString());
        *done = false;
    }
}

// Calculation of the route through the minimum spanning tree
void AdjazenzParent::calcTreeRoute() {
    int16_t currentWeight;
    unsigned int i, j, nTiefe;
    bool verbose =  termFilterPtr_->isNotFiltered(TermContextFilter::filterRoutingInfo);
    bool lRouteFailed = false;

    calcMask.clear();
    if (verbose) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                TX::getText(TX::NumberOfVertices), verticesAmount);
        termFilterPtr_->println(Global2::OutBuff);
    }
    for (i = 0; i < AdjazenzMatrixGraph::maxMatrixIdx; i++) {
        for (j = 0; j < AdjazenzMatrixGraph::maxMatrixIdx; j++) {
            currentWeight = myGraphPtr->getEdgeByIndex(i, j).getWeight();
            if (currentWeight > 0) {
                calcMask.set(j);
                if (verbose) {
                    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                            TX::getText(TX::WeightOfVertex), i, j,
                            currentWeight);
                    printAndWaitUntilDone();
                }
            }
        }
    }

    /***
     if (pRouteList) {
     // alte Routen-Liste löschen
     pRouteList->~List();
     pRouteList= NULL;
     }
     if (pRouteList==NULL) {
     pRouteList = new List<AdjazenzParent::tuple>();
     }
     ***/
    routeListEntries = 0;

    // Determine route
    nTiefe = 0;
    i = 0;      // coordinator
    for (j = 0; j < verticesAmount && isNewTreeRoutePossible(); j++) {
        if (calcMask.isSet(j)) {
            currentWeight = myGraphPtr->getEdgeByIndex(i, j).getWeight();
            if (currentWeight > 0) {
                if (verbose) {
                    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                            TX::getText(TX::RouteFromToWithDepth),
                            routeListEntries, i, j, nTiefe);
                    printAndWaitUntilDone();
                }
                if (!addTreeRoute((uint16_t) i, (uint16_t) j)) {
                    lRouteFailed = true;
                    break;
                }
                calcMask.unSet(i);
                calcMask.unSet(j);

                calcTRHelper((uint16_t) j, (uint16_t) i,
                        (uint16_t) (nTiefe + 1));
                if (verbose) {
                    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                            TX::getText(TX::RouteFromToWithDepth),
                            routeListEntries, j, i, nTiefe);
                    printAndWaitUntilDone();
                }
                addTreeRoute((uint16_t) j, (uint16_t) i);
                // break;  // 21.02.2024
            }
        }
    }  // for j

    if (lRouteFailed) {
        if (verbose) {
            termFilterPtr_->println(TX::RouteMaxLenReached);
        }
        routeFailed = true;
    } else if (calcMask.getAllMask() != 0) {
        if (verbose) {
            char temp[10];
            termFilterPtr_->println(TX::SpanningTreeIsCleaved);
            Global2::OutBuff[0] = 0;
            for (i = 0; i <= verticesAmount; i++) {
                if (calcMask.isSet(i)) {
                    if (strlen(Global2::OutBuff) > 0) {
                      strncat(Global2::OutBuff, ",", sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
                    }
                    snprintf(temp, sizeof(temp), "%3d", i);
                    strncat(Global2::OutBuff, temp, sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
                }
            }
            printAndWaitUntilDone();
        }
        routeFailed = true;
    } else {
        routeFailed = false;
    }
}

// Helping function to determine the route through the tree
// Return: True: the way back has to be determined
bool AdjazenzParent::calcTRHelper(uint16_t next, uint16_t back,
        uint16_t nTiefe) {
    if (nTiefe > verticesAmount) {
        return false;
    }
    bool treffer = false;
    bool verbose =  termFilterPtr_->isNotFiltered(TermContextFilter::filterRoutingInfo);
    int16_t currentWeight;
    unsigned int i, j;
    i = next;
    // if(i == 18){
    //   asm("nop");
    // }
    WatchDog::feed();
    for (j = 0; j < verticesAmount && isNewTreeRoutePossible(); j++) {
        currentWeight = myGraphPtr->getEdgeByIndex(i, j).getWeight();
        if ((currentWeight > 0) && (j != back) && calcMask.isSet(j)) {
            if (verbose) {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                        TX::getText(TX::RouteFromToWithDepth), routeListEntries,
                        i, j, nTiefe);
                termFilterPtr_->println(Global2::OutBuff);
            }
            if (!addTreeRoute((uint16_t) i, (uint16_t) j)) {
                break;
            }
            calcMask.unSet(i);
            calcMask.unSet(j);

            treffer = true;
            calcTRHelper((uint16_t) j, (uint16_t) i, (uint16_t) (nTiefe + 1));
            if (verbose) {
                snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
                        TX::getText(TX::RouteFromToWithDepth), routeListEntries,
                        j, i, nTiefe);
                printAndWaitUntilDone();
            }
            addTreeRoute((uint16_t) j, (uint16_t) i);
        }
    }
    return !treffer;
}

bool AdjazenzParent::addTreeRoute(uint16_t source, uint16_t destination) {
    tuple newRoute;
    newRoute.source = source;
    newRoute.destination = destination;
    if (isNewTreeRoutePossible()) {
        aRouteList[routeListEntries++] = newRoute;  // pRouteList->add(newRoute);
        return true;
    } else {
        return false;
    }
}

bool AdjazenzParent::isNewTreeRoutePossible() {
    return routeListEntries < kRouteListSize;
}

// Determines the last index of the route for an alive round trip
// Background: There is no need to traverse a return route to the coordinator
// from the last peer in the minimum spanning tree
void AdjazenzParent::retrieveLastRouteIndex(void) {
    uint16_t result = 0, d;
    uint32_t i, j;
    bool lHit;

    for (i = 0; i < routeListEntries; i++) {
        d = aRouteList[i].destination;
        if (d > 0) {
            lHit = false;
            for (j = 0; j < i; j++) {
                if (d == aRouteList[j].destination) {
                    lHit = true;
                    break;
                }
            }
            if (!lHit) {
                result = (uint16_t) i;
            }
        }
    }
    lastRouteIndex = result;
}

void AdjazenzParent::showRouteLine(char *output, int outLen, bool *done) {
    /***
     tuple *aRoute = &pRouteList->getIndexer()->getData();
     if (aRoute) {
     sprintf(output, "Route %d -> %d", aRoute->source, aRoute->destination);
     pRouteList->loopIndexer();
     *done = false;
     } else {
     *done = true;
     }
     ***/
    bool not_done = routeListDisplayIndex < getRouteListEntries(false);
    if (not_done) {
        tuple *aRoute = &aRouteList[routeListDisplayIndex];
        char shortend = ' ';
        if (routeListDisplayIndex == lastRouteIndex) {
            shortend = '*';
        }
        snprintf(output, outLen, TX::getText(TX::RouteFromTo), aRoute->source,
                aRoute->destination, shortend);
        routeListDisplayIndex++;
    } else {
        *output = 0;
    }
    *done = !not_done;
}

uint16_t AdjazenzParent::getMaxSource() {
    uint16_t result = 0;
    for (uint16_t i = 0; i < routeListEntries; i++) {
        tuple *aRoute = &aRouteList[i];
        if (aRoute->source > result) {
            result = aRoute->source;
        }
    }
    return result;
}

// Shorten list of the route, that there is no return to any node of a branch.
#pragma GCC diagnostic ignored "-Wstack-usage="
void AdjazenzParent::shortenRouteList() {
    tuple copyRouteList[kRouteListSize];  // Variable is on the stack.
                                          // May be it is necessary to move it to the heap
    tuple nt;
    uint16_t copyRouteListEntries = routeListEntries;   // lastRouteIndex;
    BitMask<uint32_t> usedSource;
    uint32_t i, j;

    // 1) Create a copy of the original list
    memcpy(copyRouteList, aRouteList, sizeof(tuple) * kRouteListSize);

    // 2) Delete old list
    routeListEntries = 0;

    // 3) Transfer entries from the copy to the original list and pay attention to them
    // that a return to single nodes is suppressed
    usedSource.clear();
    i = 0;
    while (i < copyRouteListEntries) {
        tuple *aRoute = &copyRouteList[i];
        uint32_t s = aRoute->source & 0x1F;         // ToDo(AME): Implement variant for more than 32 bits
        uint32_t d = aRoute->destination & 0x1F;    //      "       "
        nt.source = aRoute->source;
        if (usedSource.isSet(s) || usedSource.isSet(d)) {
            for (j = i + 1; j < copyRouteListEntries; j++) {
                aRoute = &copyRouteList[j];
                s = aRoute->source & 0x1F;
                if (!usedSource.isSet(s)) {
                    nt.destination = aRoute->source;
                    usedSource.set(s);
                    addTreeRoute(nt.source, nt.destination);
                    nt.source = nt.destination;
                }
            }
            nt.destination = 0;
            addTreeRoute(nt.source, nt.destination);
            break;
        } else {
            nt.destination = aRoute->destination;
            usedSource.set(s);
            addTreeRoute(nt.source, nt.destination);
        }
        i++;
    }

    routeListDisplayIndex = 0;
    retrieveLastRouteIndex();
}

void AdjazenzParent::showMatrixOnTerminal() {
    int16_t currentWeight;
    char temp[10];
    unsigned int i, j;

    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::AdjacencyMatrix));
    printAndWaitUntilDone();

    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "   |");  // Vertex top, left
    for (i = 0; i < verticesAmount; i++) {
        snprintf(temp, sizeof(temp), "%3d|", i);
        strncat(Global2::OutBuff, temp, sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
    }
    printAndWaitUntilDone();

    snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "---+");  // underline first row
    snprintf(temp, sizeof(temp), "---+");
    for (i = 0; i < verticesAmount; i++) {
        strncat(Global2::OutBuff, temp, sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
    }
    printAndWaitUntilDone();

    for (i = 0; i < verticesAmount; i++) {
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%3d|", i);
        for (j = 0; j < verticesAmount; j++) {
            currentWeight = myGraphPtr->getEdgeByIndex(i, j).getWeight();
            if (currentWeight > 0) {
                snprintf(temp, sizeof(temp), "%3d|", currentWeight);
            } else {
                snprintf(temp, sizeof(temp), "   |");
            }
            strncat(Global2::OutBuff, temp, sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
        }
        printAndWaitUntilDone();

        // Underline row n
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "---+");
        snprintf(temp, sizeof(temp), "---+");
        for (j = 0; j < verticesAmount; j++) {
            strncat(Global2::OutBuff, temp, sizeof(Global2::OutBuff) - strlen(Global2::OutBuff) - 1);
        }
        printAndWaitUntilDone();
    }
}

void AdjazenzParent::printAndWaitUntilDone() {
    termFilterPtr_->println(Global2::OutBuff);
    while (!termFilterPtr_->isTransmissionComplete()) {
        WatchDog::feed();
        termFilterPtr_->popTxD();
        for (int i = 1; i < 1000; i++) {
            asm("nop");
        }
    }
}


// Set route by terminal command
bool AdjazenzParent::setRoute(const char *input) {
    unsigned int cnt, val1;
    uint16_t source = 0;
    routeListEntries = 0;
    do {
        cnt = sscanf(input, "%u", &val1);
        if (cnt == 1) {
            if (val1 <= 1000) {
                if (source != val1) {
                    addTreeRoute(source, (uint16_t) val1);
                }
                source = (uint16_t) val1;
                input = charOperations::skip_token(input, ' ');
            } else {
                return false;
            }
        }
    } while (cnt == 1);
    if (source) {
        addTreeRoute(source, 0);
    }

    return true;
}

}  // namespace AME_SRC
