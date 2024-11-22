/*
 * MinSpanTreeCalculator.h
 *
 *  Created on: 25.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_MINSPANTREECALCULATOR_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_MINSPANTREECALCULATOR_H_

#include "IMinSpanTreeCalculator.h"

#include <cstdint>

namespace AME_SRC {

class MinSpanTreeCalculator: public IMinSpanTreeCalculator {
 public:
    MinSpanTreeCalculator();
    void loadData(IGraph *graph);
    IGraph* compute();
    virtual ~MinSpanTreeCalculator();

 private:
    IGraph *graphPtr;
    enum {
        dim_src = 100
    };
    int16_t selectedEdges_[dim_src][2];  // [0] := src // [1] := dst
    enum { src, dst };
    uint16_t verticesAmount_;
    int16_t selectionIndex_;
    int16_t smallestEdgeLocation_[2];    // [0] := src // [1] := dst
    int16_t smallestWeight_;

    // static Edge*edgeArray[200];
    void primAlgorithm();
    void init();
    void updateSmallestEdge();
    void compareCurrentWeightForVertex(int16_t currentWeight,
            uint16_t vertexID);
    bool isNodeIndexSelected(int16_t nodeIndex);
    void trimShortBranches();
    void clearUnselectedEdges();
    bool isEdgeLocationNotSelected(int16_t srcVertexIndex, int16_t destVertexIndex);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_MINSPANTREECALCULATOR_H_
