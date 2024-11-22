/*
 * Edge.h
 *
 *  Created on: 25.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_EDGE_H_
#define SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_EDGE_H_

#include <cstdint>

namespace AME_SRC {

class Edge {
 public:
    Edge();
    Edge(int16_t dest, int16_t weight);
    int16_t getDestination() const;
    int16_t getWeight() const;
    void setWeight(int16_t value);

 private:
    int16_t destination_;
    int16_t weight_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_COMPOSITECOMPONENTS_FOUNDATIONMODULE_EDGE_H_
