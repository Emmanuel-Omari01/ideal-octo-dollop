/*
 * IRandGenerator.h
 *
 *  Created on: 05.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_IRANDGENERATOR_H_
#define SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_IRANDGENERATOR_H_

namespace AME_SRC {

class IRandGenerator {
 public:
    virtual void setRange(int lowBoard, int highBoarder) = 0;
    virtual void setSeed(int seed) = 0;
    virtual int generate() = 0;
    virtual ~IRandGenerator() = default;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_IRANDGENERATOR_H_
