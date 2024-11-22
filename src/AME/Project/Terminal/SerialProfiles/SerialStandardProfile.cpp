/*
 * SerialStandardProfile.cpp
 *
 *  Created on: 06.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "SerialStandardProfile.h"

#include <cstring>

namespace AME_SRC {

SerialStandardProfile::SerialStandardProfile() {
	cmdLines.indexIN = 0;
	cmdLines.indexOUT = 0;
}

SerialStandardProfile::~SerialStandardProfile() = default;

void SerialStandardProfile::interpretReceivedLetter(char letter) {
	static 	char currentCmd[cmdLength];
	bool lCopy = 0;
	static uint16_t rxd_index = 0;
	switch (letter ) {
		case ETB:
			currentCmd[rxd_index++] = letter;
			// No break
		case RETURN:
		case ETX:
			lCopy = 1;
			break;
		case BACKSPACE:
		case DEL:
			if (rxd_index)
				rxd_index--;
			break;
		case LINEFEED:
			rxd_index=0;			// beginne von vorn
			break;
		case ESC:
		case STX:
			rxd_index=0;		// beginne von vorn.
			break;
		case SOH:
			rxd_index=0;		// beginne von vorn.
			// No break
		default:
			currentCmd[rxd_index++] = letter;
			break;
	}
	if (lCopy) {
		currentCmd[rxd_index] = 0;	// Zeile terminieren
		fillNextCommandWithString(currentCmd);
		rxd_index = 0;
	}

}

void SerialStandardProfile::fillNextCommandWithString(char *targetStringPtr) {

			char *destinationPtr;
			destinationPtr = &cmdLines.container[cmdLines.indexIN][0];
			memcpy(destinationPtr, targetStringPtr, cmdLength); 	// Zeichen für die Auswertung umkopieren
			memset(targetStringPtr, 0, cmdLength);		// doppelte Interpretation durch alte Werte verhindern
			// Interpreter triggern
			if (++cmdLines.indexIN>=cmdCount)
				cmdLines.indexIN = 0;

}

char *SerialStandardProfile::getBuffer() {
	char * p;
	if (cmdLines.indexOUT != cmdLines.indexIN) {
		p=cmdLines.container[cmdLines.indexOUT];
		// Ringspeicher-Zeiger korrigieren
		if (++cmdLines.indexOUT>=cmdCount){
			cmdLines.indexOUT = 0;
		}
		return p;
	} else
		return 0;
}

void SerialStandardProfile::setBuffer(uint8_t count, char *array) {
	fillNextCommandWithString(array);
}

}  // namespace AME_SRC
