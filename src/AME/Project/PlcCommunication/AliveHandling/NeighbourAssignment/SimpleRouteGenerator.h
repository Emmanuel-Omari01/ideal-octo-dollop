/*
 * SimpleRouteGenerator.h
 *
 *  Created on: Dec 1, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_SIMPLEROUTEGENERATOR_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_SIMPLEROUTEGENERATOR_H_

#include "IMinSpanTreeCalculator.h"

namespace AME_SRC {

class SimpleRouteGenerator: public IMinSpanTreeCalculator {
 public:
    SimpleRouteGenerator();
    virtual ~SimpleRouteGenerator();

    void loadData(IGraph *graph);
    IGraph* compute();
 private:
    static const uint32_t matrixSize = 30;
    bool connectedNodes_[matrixSize];
    IGraph *graphPtr;
    void markConnectedNodes();
    void fillGraphWithNodeConnections();
    uint8_t countNotConnectedNodesFromIndex(uint8_t index);
};

}  // namespace AME_SRC

#endif /* SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_SIMPLEROUTEGENERATOR_H_ */
