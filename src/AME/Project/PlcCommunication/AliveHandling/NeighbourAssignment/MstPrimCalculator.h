/*
 * MstPrimCalculator.h
 *
 *  Created on: Aug 23, 2023
 *  Author: D. Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_MSTPRIMCALCULATOR_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_MSTPRIMCALCULATOR_H_

#include <cstdint>

#include "IMinSpanTreeCalculator.h"
#include "AdjazenzMatrixGraph.h"

namespace AME_SRC {

#define V AdjazenzMatrixGraph::maxMatrixIdx

class MstPrimCalculator {
 public:
    MstPrimCalculator();
    void loadData(IGraph *graph);
    IGraph* compute();
    virtual ~MstPrimCalculator();
 private:
    IGraph *graphPtr;
    uint16_t minKey(uint16_t key[], bool mstSet[], uint16_t vMax);
    enum { MaxVerticalVal = 0xFFFF };
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_MSTPRIMCALCULATOR_H_
