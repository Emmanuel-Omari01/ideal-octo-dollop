/*
  * IMinSpanTreeCalculator.h
 *
 *  Created on: 25.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IMINSPANTREECALCULATOR_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IMINSPANTREECALCULATOR_H_

#include "IGraph.h"

#include <cstdint>

namespace AME_SRC {

class IMinSpanTreeCalculator {  // @suppress("Class has a virtual method and non-virtual destructor")
 public:
    virtual void loadData(IGraph *graph) = 0;
    virtual IGraph* compute() = 0;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_NEIGHBOURASSIGNMENT_IMINSPANTREECALCULATOR_H_
