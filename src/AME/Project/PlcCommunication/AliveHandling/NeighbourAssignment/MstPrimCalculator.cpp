/*
 * MstPrimCalculator.cpp
 *
 *  Created on: Aug 23, 2023
 *      Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

// Alternative calculation of a minimum spanning tree
// PLEASE DO NOT INCLUDE THIS CLASS - IS  NOT CHECKED

#include "MstPrimCalculator.h"

namespace AME_SRC {

MstPrimCalculator::MstPrimCalculator():graphPtr(NULL) {
    // TODO(AME) Auto-generated constructor stub
}

void MstPrimCalculator::loadData(IGraph* graph) {
    graphPtr = graph;
}

#pragma GCC diagnostic ignored "-Wstack-usage="
IGraph* MstPrimCalculator::compute() {
    int resultMst[V];   // Array to store constructed MST
    uint16_t key[V];    // Key values used to pick minimum weight edge in cut
    bool mstSet[V];     // To represent set of vertices included in MST
    unsigned int i, v;
    uint16_t count;

    // Initialize all keys as INFINITE
    for (i = 0; i < V; i++) {
        key[i] = MaxVerticalVal;
        mstSet[i] = false;
    }

    // Always include first 1st vertex in MST.
    // Make key 0 so that this vertex is picked as first vertex.
    key[0] = 0;
    resultMst[0] = -1;                                              // First node is always root of MST
    uint16_t verticesAmount = (uint16_t) graphPtr->getVertices();   // The MST will have verticesAmount vertices

    for (count = 0; count < verticesAmount - 1; count++) {
        // Pick the minimum key vertex from the set of vertices not yet included in MST
        uint16_t u = minKey(key, mstSet, verticesAmount);
        mstSet[u] = true;   // Add the picked vertex to the MST Set

        // Update key value and resultMst index of the adjacent vertices of the picked vertex.
        // Consider only those vertices which are not yet included in MST
        for (v = 0; v < verticesAmount; v++) {
            // graph[u][v] is non zero only for adjacent vertices of m mstSet[v] is false for vertices
            // not yet included in MST Update the key only if graph[u][v] is smaller than key[v]
            int16_t w = graphPtr->getEdgeByIndex(u, v).getWeight();
            if (w && mstSet[v] == false && w < key[v]) {
                resultMst[v] = u;
                key[v] = w;
            }
        }
    }

    // Copy the results to the source matrix
    for (i = 0; i < verticesAmount; i++) {
        for (v = 0; v < verticesAmount; v++) {
            if (resultMst[v]) {
                graphPtr->getEdgeByIndex(i, v).setWeight(key[v]);
            } else {
                graphPtr->getEdgeByIndex(i, v).setWeight(0);
            }
        }
    }
    return graphPtr;
}

MstPrimCalculator::~MstPrimCalculator() = default;

// Function to find edge with a minimum value, which are not part of the spanning tree
uint16_t MstPrimCalculator::minKey(uint16_t key[], bool mstSet[], uint16_t vMax) {
    // Initialize min & return value
    int minVal = MaxVerticalVal;
    int16_t minIndex = 0;

    for (int v = 0; v < vMax; v++) {
        if (mstSet[v] == false && key[v] < minVal) {
            minVal = key[v];
            minIndex = (uint16_t) v;
        }
    }
    return minIndex;
}

}  // namespace AME_SRC
