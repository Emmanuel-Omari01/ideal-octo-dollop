/*
 * IGraph.h
 *
 *  Created on: 25.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IGRAPH_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IGRAPH_H_

#include <cstdint>

#include "../../../CompositeComponents/FoundationModule/Vector.h"
#include "../../../CompositeComponents/FoundationModule/Edge.h"

namespace AME_SRC {

class IGraph  {  // @suppress("Class has a virtual method and non-virtual destructor")
 public:
    virtual void clearMatrix() = 0;
    virtual void addEdge(int src, int dest, int16_t weight) = 0;
    virtual Edge& getEdgeByIndex(int src, int dest) = 0;
    virtual const char* toString() = 0;
    virtual bool isDisplayed() = 0;
    virtual int getVertices() = 0;
    virtual int getEdgesForVertexID(int id) = 0;
    virtual void* getMatrixOrigin() = 0;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IGRAPH_H_
