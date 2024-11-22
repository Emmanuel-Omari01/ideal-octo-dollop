/*
 * AdjazenzMatrixGraph.cpp
 *
 *  Created on: 26.05.2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "AdjazenzMatrixGraph.h"

#include <cstring>

namespace AME_SRC {

AdjazenzMatrixGraph::AdjazenzMatrixGraph() :
        displayIndex(0), displayed(false) {
}

AdjazenzMatrixGraph::~AdjazenzMatrixGraph() = default;

// Delete matrix to be able to call the class multiple times if necessary
void AdjazenzMatrixGraph::clearMatrix() {
    memset(matrix, 0, maxMatrixIdx * maxMatrixIdx * sizeof(Edge));
    displayIndex = 0;
    displayed = false;
}

void AdjazenzMatrixGraph::addEdge(int src, int dest, int16_t weight) {
    matrix[src][dest] = Edge(dest, weight);
}

Edge& AdjazenzMatrixGraph::getEdgeByIndex(int src, int dest) {
    return matrix[src][dest];
}

uint16_t AdjazenzMatrixGraph::getWightByIndex(int src, int dest) const {
    return matrix[src][dest].getWeight();
}

const char* AdjazenzMatrixGraph::toString() {
    const char *output;
    RowDispInfo *info = calculateDispIndex();
    output = getPresentationFromRowDispInfo(*info);
    return output;
}

const char* AdjazenzMatrixGraph::getPresentationFromRowDispInfo(RowDispInfo dispInfo) {
    static uint16_t currentEdgeCount = 0;
    const char *sideRowFormat = " +---{%2d}-->[%d]";
    const char *middleRowFormat = "[%d]--{%2d}-->[%d]";
    const char *endFormat = "\r:------------:\r";
    if (displayIndex == 0 && currentEdgeCount != 0) {
        currentEdgeCount = 0;
        displayIndex = -1;
        return endFormat;
    }
    Edge targetEdge = getEdgeByIndex(dispInfo.rootEdgeIndex, displayIndex);
    if (targetEdge.getWeight() != 0
            && currentEdgeCount <= dispInfo.validEdgeCount
            && dispInfo.rootEdgeIndex != targetEdge.getDestination()) {
        if (currentEdgeCount == 0) {
            sprintf(dispInfo.outputPtr, middleRowFormat, dispInfo.rootEdgeIndex,
                    targetEdge.getWeight(), targetEdge.getDestination());
        } else {
            sprintf(dispInfo.outputPtr, " |\r");
            sprintf(dispInfo.outputPtr + 3, sideRowFormat,
                    targetEdge.getWeight(), targetEdge.getDestination());
        }
        currentEdgeCount++;
    } else {
        *dispInfo.outputPtr = 0;    // dispInfo.outputPtr = "";
    }
    return dispInfo.outputPtr;
}

bool AdjazenzMatrixGraph::isDisplayed() {
    return displayed;
}

int AdjazenzMatrixGraph::getVertices() {
    int colAmount = sizeof(matrix)[0] / sizeof(matrix)[0][0];
    uint16_t vertexMax = 0;  // vertexCount
    for (uint16_t colID = 0; colID < colAmount; colID++) {
        if (getEdgesForVertexID(colID) > 0) {
            vertexMax = (uint16_t) (colID + 1);  // vertexCount++;
        }
    }
    return vertexMax;
}

int AdjazenzMatrixGraph::getEdgesForVertexID(int id) {
    int rowLength = sizeof(matrix)[0] / sizeof(matrix)[0][0];
    uint16_t edgeCount = 0;
    for (int i = 0; i < rowLength; i++) {
        if (matrix[id][i].getWeight() > 0) {
            edgeCount++;
        }
    }
    return edgeCount;
}

AdjazenzMatrixGraph::RowDispInfo* AdjazenzMatrixGraph::calculateDispIndex() {
    static char output[64];
    static RowDispInfo info = { 0, 0, output };
    int rowLength = sizeof(matrix)[0] / sizeof(matrix)[0][0];
    if (displayIndex == 0) {
        for (int j = 0; j < (rowLength); j++) {
            int edgeWeight = getEdgeByIndex(info.rootEdgeIndex, j).getWeight();
            if (edgeWeight != 0) {
                info.validEdgeCount++;
            }
        }
    }
    if (++displayIndex >= rowLength) {
        displayIndex = 0;
        info.validEdgeCount = 0;
        if (++info.rootEdgeIndex > (rowLength - 1)) {
            displayed = true;
            *(info.outputPtr) = 0;      // info.outputPtr = "";
            info.rootEdgeIndex = 0;
        }
    }
    return &info;
}

}  // namespace AME_SRC
