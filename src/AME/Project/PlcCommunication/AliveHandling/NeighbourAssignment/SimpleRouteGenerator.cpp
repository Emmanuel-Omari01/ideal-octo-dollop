/*
 * SimpleRouteGenerator.cpp
 *
 *  Created on: Dec 1, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "SimpleRouteGenerator.h"

#include <cstring>

namespace AME_SRC {

SimpleRouteGenerator::SimpleRouteGenerator(): connectedNodes_{0}, graphPtr(NULL) {
}

SimpleRouteGenerator::~SimpleRouteGenerator() = default;

void SimpleRouteGenerator::loadData(IGraph* graph) {
    graphPtr = graph;
}

IGraph* SimpleRouteGenerator::compute() {
    uint32_t i, j;

    markConnectedNodes();
    graphPtr->clearMatrix();
    fillGraphWithNodeConnections();

    return graphPtr;
}

void SimpleRouteGenerator::markConnectedNodes() {
    uint32_t rowIndex, colIndex;
    for (rowIndex = 0; rowIndex < matrixSize; rowIndex++) {
        for (colIndex = rowIndex + 1; colIndex < matrixSize; colIndex++) {
            Edge &selectedEdge = graphPtr->getEdgeByIndex(rowIndex, colIndex);
            if (selectedEdge.getWeight() > 0) {
                connectedNodes_[rowIndex] = true;
                connectedNodes_[colIndex] = true;
            }
        }
    }
}

void SimpleRouteGenerator::fillGraphWithNodeConnections() {
    uint8_t nodeIndex, nodeGapWidth;
    for (nodeIndex = 0; nodeIndex < matrixSize - 1; nodeIndex++) {
        nodeGapWidth = countNotConnectedNodesFromIndex(nodeIndex + 1);
        if (connectedNodes_[nodeIndex]) {
            graphPtr->addEdge(nodeIndex, nodeIndex + nodeGapWidth + 1, 1);
        }
    }
}

uint8_t SimpleRouteGenerator::countNotConnectedNodesFromIndex(uint8_t index) {
    uint8_t count = 0;
    while (!connectedNodes_[index + count] && index < matrixSize) {
        count++;
    }
    return count;
}

}  // namespace AME_SRC

