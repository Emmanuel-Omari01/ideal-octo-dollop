/*
 * AdjazenzMatrixGraph.h
 *
 *  Created on: 26.05.2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ADJAZENZMATRIXGRAPH_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ADJAZENZMATRIXGRAPH_H_

#include "IGraph.h"

#include <cstdint>

namespace AME_SRC {

class AdjazenzMatrixGraph: public IGraph {
 public:
    enum {
        maxMatrixIdx = 30
    };
    AdjazenzMatrixGraph();
    virtual ~AdjazenzMatrixGraph();
    void clearMatrix() override;
    void addEdge(int src, int dest, int16_t weight) override;
    Edge& getEdgeByIndex(int src, int dest) override;
    uint16_t getWightByIndex(int src, int dest) const;
    const char* toString() override;
    bool isDisplayed() override;
    int getVertices() override;
    int getEdgesForVertexID(int id) override;
    void* getMatrixOrigin() override {
        return &matrix;
    }

 private:
    Edge matrix[maxMatrixIdx][maxMatrixIdx];
    int displayIndex;
    bool displayed;

    typedef struct {
        uint16_t validEdgeCount;    // Amount of Max relevant displayed Edges
        uint16_t rootEdgeIndex;     // Current Root of the Displayed Subgraph
        char *outputPtr;
    } RowDispInfo;

    const char* getPresentationFromRowDispInfo(RowDispInfo dispInfo);
    RowDispInfo* calculateDispIndex();
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_ADJAZENZMATRIXGRAPH_H_
