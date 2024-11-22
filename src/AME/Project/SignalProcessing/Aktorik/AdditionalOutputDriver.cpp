/*
 * AdditionalOutputDriver.cpp
 *
 *  Created on: 09.07.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "AdditionalOutputDriver.h"
#include "../../SystemBase/ProgramConfig.h"
#include "../../HelpStructures/CharOperations.h"
#include "../../SystemLogic/I18N.h"

namespace AME_SRC {


AdditionalOutputDriver::AdditionalOutputDriver() = default;

AdditionalOutputDriver::~AdditionalOutputDriver() = default;

/***
 * This method switches according to a passed index 1 of the 4 relays
 * @return message to the switching process
 */
char* AdditionalOutputDriver::callRelayToggle(char *commandArguments) {
    uint8_t relayIndex = charOperations::commandArgumentToNumber(commandArguments);
    IoPin *selectedRelay = getRelayPinByIndex(relayIndex);
  if (PCBPortSelector::isValidPin(selectedRelay)) {
        selectedRelay->toogle();
        return const_cast<char*>(TX::getText(TX::TheRelaySwitched));
    } else {
        return const_cast<char*>(TX::getText(TX::IncorrectIndexEntry));
    }
}

/***
 * This method chooses between one of the four relais selected by an index
 * @param relayIndex index of the required relay
 * @return pointer to the relay object
 */
IoPin* AdditionalOutputDriver::getRelayPinByIndex(const uint8_t relayIndex) {
    IoPin *selectedRelay = PCBPortSelector::getPin(PCBPortSelector::ModuleOutputPin, relayIndex);

    return selectedRelay;
}

}  // namespace AME_SRC
