/*
 * Edge.cpp
 *
 *  Created on: 25.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "Edge.h"

namespace AME_SRC {

Edge::Edge(int16_t dest, int16_t weight) : destination_(dest), weight_(weight) {}
Edge::Edge() : destination_(0), weight_(0) {}


int16_t Edge::getDestination() const {
  return destination_;
}

int16_t Edge::getWeight() const {
  return weight_;
}

void Edge::setWeight(int16_t value) {
    weight_ = value;
}

}  // namespace AME_SRC
