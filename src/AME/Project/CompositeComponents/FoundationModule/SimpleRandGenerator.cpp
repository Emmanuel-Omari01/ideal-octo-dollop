/*
 * SimpleRandGenerator.cpp
 *
 *  Created on: 05.05.2023
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "SimpleRandGenerator.h"

#include <cstdlib>

namespace AME_SRC {

SimpleRandGenerator::SimpleRandGenerator(int lowBoard, int highBoarder, int seed) {
    setRange(lowBoard, highBoarder);
    setSeed(seed);
}

SimpleRandGenerator::SimpleRandGenerator() {
    setRange(0, 1);
    setSeed(0);
}

void SimpleRandGenerator::setRange(int lowBoard_, int highBoarder_) {
    this->lowBoard = lowBoard_;
    this->highBoarder = highBoarder_;
}

void SimpleRandGenerator::setSeed(int seed_) {
    this->seed = seed_;
}

int SimpleRandGenerator::generate() {
    srand(seed);
    int range = (highBoarder - lowBoard);
    int random_number = rand() % range + lowBoard;
    return random_number;
}

SimpleRandGenerator::~SimpleRandGenerator() = default;

}  // namespace AME_SRC
