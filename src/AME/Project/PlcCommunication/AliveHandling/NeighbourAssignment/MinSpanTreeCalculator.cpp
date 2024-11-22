/*
 * MinSpanTreeCalculator.cpp
 *
 *  Created on: 25.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "MinSpanTreeCalculator.h"

#include <cstdlib>

namespace AME_SRC {

MinSpanTreeCalculator::MinSpanTreeCalculator() :
        graphPtr(NULL), selectionIndex_(0), smallestWeight_(-1), verticesAmount_(0) {
    init();
    smallestEdgeLocation_[src] = -1;
    smallestEdgeLocation_[dst] = -1;
}

void MinSpanTreeCalculator::loadData(IGraph *graph) {
    graphPtr = graph;
    verticesAmount_ = graphPtr->getVertices();
//  int vertices = graph.getVertices();
//  int edgeArrayCounter = 0;
//  for(int vertexID = 0 ; vertexID<vertices; vertexID++) {
//      int edgeAmount = graph.getEdgesForVertexID(vertexID);
//      for(int edgeID=0;edgeID<edgeAmount;edgeID++){
//          Edge& targetEdge = graph.getEdgeByIndex(vertexID, edgeID);
//          if (targetEdge.getWeight() > 0) {
//              edgeArray[edgeArrayCounter++] = & targetEdge;
//          }
//      }
//  }
}

IGraph* MinSpanTreeCalculator::compute() {
    primAlgorithm();
    // trimShortBranches();
    clearUnselectedEdges();
    return graphPtr;
}

MinSpanTreeCalculator::~MinSpanTreeCalculator() = default;

/***
The Prim algorithm is a greedy algorithm that can be used for weighted
 and connected graphs can determine the maximum spanning tree.
 The algorithm is used to find the shortest path from a specific starting point
 to find a specific end point in a graph. In the Prim algorithm,
 starting from a random point, the next edge with the lowest one
 Added edge weight until a minimal spanning tree is created.
 The Prim algorithm is one of the greedy algorithms in graph theory.
*/
void MinSpanTreeCalculator::primAlgorithm() {
    init();
    for (uint16_t srcVertexIndex = 0; srcVertexIndex < verticesAmount_;
            srcVertexIndex++) {
        smallestEdgeLocation_[src] = -1;
        smallestEdgeLocation_[dst] = -1;
        smallestWeight_ = -1;
        if (srcVertexIndex == 0) {
            selectionIndex_ = 0;
            updateSmallestEdge();
            selectedEdges_[0][src] = smallestEdgeLocation_[src];
            selectedEdges_[0][dst] = smallestEdgeLocation_[dst];
            selectionIndex_ = -1;
            smallestWeight_ = -1;
            continue;
        }
        for (selectionIndex_ = 0; selectionIndex_ < verticesAmount_;
                selectionIndex_++) {
            updateSmallestEdge();
        }
        selectedEdges_[srcVertexIndex][src] = smallestEdgeLocation_[src];
            // TODO(AME): store src index if index found (delivery of src index)
        selectedEdges_[srcVertexIndex][dst] = smallestEdgeLocation_[dst];
    }
}

/*
 * reset all visited edges (-1)
 */
void MinSpanTreeCalculator::init() {
    for (int i = 0; i < dim_src; selectedEdges_[i++][src] = -1) {
        selectedEdges_[i][dst] = -1;
    }
}

void MinSpanTreeCalculator::updateSmallestEdge() {
    if (isNodeIndexSelected(selectionIndex_)) {
        for (uint8_t destVertexIndex = 0; destVertexIndex < verticesAmount_;
                destVertexIndex++) {
            if (!isNodeIndexSelected(destVertexIndex)) {
                int16_t currentWeight = graphPtr->getEdgeByIndex(
                        selectionIndex_, destVertexIndex).getWeight();
                compareCurrentWeightForVertex(currentWeight, destVertexIndex);
            }
        }
    }
}

void MinSpanTreeCalculator::compareCurrentWeightForVertex(int16_t currentWeight,
        uint16_t vertexID) {
    if (currentWeight > 0) {
        bool isweightNotSet = (smallestWeight_ == -1);
        bool isCurrentWeightSmallest = (currentWeight <= smallestWeight_);
        if (isweightNotSet || isCurrentWeightSmallest) {
            smallestEdgeLocation_[src] = selectionIndex_;
            smallestEdgeLocation_[dst] = vertexID;
            smallestWeight_ = currentWeight;
        }
    }
}

bool MinSpanTreeCalculator::isNodeIndexSelected(int16_t nodeIndex) {
    if (nodeIndex == 0) {
        return true;
    }
    for (int i = 0; i < verticesAmount_; i++) {
        if (selectedEdges_[i][src] == -1) {
            return false;
        }
        bool isDestEqual = selectedEdges_[i][dst] == nodeIndex;
        if (isDestEqual /**&& isSrcEqual**/) {  // <<----- Bug fix error in test 5!
            return true;  // Value found in array
        }
    }
    return false;  // Value not found in array
}

// if (isDestEqual /**&& isSrcEqual**/)

void MinSpanTreeCalculator::trimShortBranches() {
    bool edgeMarkings[dim_src] = { 0 };
    bool isNotEdgeOfBranch;
    bool edgesAvailable;
    uint8_t depth = 0;
    for (uint8_t depthIndexer = 0; depthIndexer < dim_src; depthIndexer++) {
        do {
            int16_t currentBranchEnd = selectedEdges_[depthIndexer][dst];
            int16_t connectingEdgeBegin = selectedEdges_[++depthIndexer][src];
            edgesAvailable = connectingEdgeBegin != -1;
            isNotEdgeOfBranch = currentBranchEnd != connectingEdgeBegin;
        } while (edgesAvailable && isNotEdgeOfBranch);
        depth++;

        if (!edgesAvailable) {
            break;
        }
    }
}

void MinSpanTreeCalculator::clearUnselectedEdges() {
    for (int srcVertexIndex = 0; srcVertexIndex < verticesAmount_;
            srcVertexIndex++) {
        for (int destVertexIndex = 0; destVertexIndex < verticesAmount_; destVertexIndex++) {
            if (isEdgeLocationNotSelected((int16_t) srcVertexIndex, (int16_t) destVertexIndex)) {
                graphPtr->getEdgeByIndex(srcVertexIndex, destVertexIndex).setWeight(
                        0);
            }
        }
    }
}

bool MinSpanTreeCalculator::isEdgeLocationNotSelected(int16_t srcVertexIndex,
        int16_t destVertexIndex) {
    for (int i = 0; i < verticesAmount_; i++) {
        bool isSrcEqual = (selectedEdges_[i][src] == srcVertexIndex);
        bool isDestEqual = (selectedEdges_[i][dst] == destVertexIndex);
        if (isSrcEqual && isDestEqual) {
            return false;
        }
    }
    return true;
}

}  // namespace AME_SRC
