/*
 * SimpleRandGenerator.h
 *
 *  Created on: 05.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_SIMPLERANDGENERATOR_H_
#define SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_SIMPLERANDGENERATOR_H_

#include "IRandGenerator.h"

namespace AME_SRC {

class SimpleRandGenerator: public IRandGenerator {
 private:
    int lowBoard;
    int highBoarder;
    int seed;
 public:
    SimpleRandGenerator(int lowBoard, int highBoarder, int seed);
    SimpleRandGenerator();
    void setRange(int lowBoard_, int highBoarder_) override;
    void setSeed(int seed_) override;
    int generate() override;
    ~SimpleRandGenerator() override;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_SIMPLERANDGENERATOR_H_
