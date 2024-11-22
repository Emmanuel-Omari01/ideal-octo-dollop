/*
 * PortManager.cpp
 *
 *  Created on: 19.01.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#include "PortManager.h"

#include <cstdint>

#include <platform.h>

namespace AME_SRC {

bool PortManager::isNotInstanciated = true;
PortManager *PortManager::singlePtr = 0;

PortManager* PortManager::getInstance() {
    if (isNotInstanciated) {
      singlePtr = new PortManager();
    }
    return singlePtr;
}

PortManager::PortManager() {
    isNotInstanciated = false;
}

PortManager::~PortManager() {
    isNotInstanciated = true;
}

void PortManager::setPinDirection(portName port, pinPosition pinPosition, pinDirection direction) {
    if (direction == Input) {
        switch (port) {
        case Port0:
            PORT0.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case Port1:
            PORT1.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case Port2:
            PORT2.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case Port3:
            PORT3.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case Port4:
            PORT4.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case PortA:
            PORTA.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case PortB:
            PORTB.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case PortC:
            PORTC.PDR.BYTE &= ~(1 << pinPosition);
            break;
        case PortE:
            PORTE.PDR.BYTE &= ~(1 << pinPosition);
            break;
        default:
            break;
        }
    } else {
        switch (port) {
        case Port0:
            PORT0.PDR.BYTE |= 1 << pinPosition;
            break;
        case Port1:
            PORT1.PDR.BYTE |= 1 << pinPosition;
            break;
        case Port2:
            PORT2.PDR.BYTE |= 1 << pinPosition;
            break;
        case Port3:
            PORT3.PDR.BYTE |= 1 << pinPosition;
            break;
        case Port4:
            PORT4.PDR.BYTE |= 1 << pinPosition;
            break;
        case PortA:
            PORTA.PDR.BYTE |= 1 << pinPosition;
            break;
        case PortB:
            PORTB.PDR.BYTE |= 1 << pinPosition;
            break;
        case PortC:
            PORTC.PDR.BYTE |= 1 << pinPosition;
            break;
        case PortE:
            PORTE.PDR.BYTE |= 1 << pinPosition;
            break;
        default:
            break;
        }
    }
}

void PortManager::setPinMode(portName port, pinPosition pinPosition, pinMode mode) {
    if (mode == GeneralIOPort) {
        switch (port) {
        case Port0:
            PORT0.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case Port1:
            PORT1.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case Port2:
            PORT2.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case Port3:
            PORT3.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case Port4:
            PORT4.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case PortA:
            PORTA.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case PortB:
            PORTB.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case PortC:
            PORTC.PMR.BYTE &= ~(1 << pinPosition);
            break;
        case PortE:
            PORTE.PMR.BYTE &= ~(1 << pinPosition);
            break;
        default:
            break;
        }
    } else {
        switch (port) {
        case Port0:
            PORT0.PMR.BYTE |= 1 << pinPosition;
            break;
        case Port1:
            PORT1.PMR.BYTE |= 1 << pinPosition;
            break;
        case Port2:
            PORT2.PMR.BYTE |= 1 << pinPosition;
            break;
        case Port3:
            PORT3.PMR.BYTE |= 1 << pinPosition;
            break;
        case Port4:
            PORT4.PMR.BYTE |= 1 << pinPosition;
            break;
        case PortA:
            PORTA.PMR.BYTE |= 1 << pinPosition;
            break;
        case PortB:
            PORTB.PMR.BYTE |= 1 << pinPosition;
            break;
        case PortC:
            PORTC.PMR.BYTE |= 1 << pinPosition;
            break;
        case PortE:
            PORTE.PMR.BYTE |= 1 << pinPosition;
            break;
        default:
            break;
        }
    }
}

// PC0/PB6 and PC1/PB7 share the same pin respectively.
// The method selects the desired functionality and should by used once for a pair of ports.
// Look at hardware manual for Port Switching Register PSRA and PSRB
// Attention: This function is needed for a 64-pin package and must be adjusted on 48-pin package.
void PortManager::setPortSwitchingRegister(portName port, pinPosition pinPosition) {
    switch (port) {
    case PortB:
        switch (pinPosition) {
        case 6:  // PB6
            PORT.PSRA.BIT.PSEL6 = 0;
            break;
        case 7:  // PB7
            PORT.PSRA.BIT.PSEL7 = 0;
            break;
        }
        break;
    case PortC:
        switch (pinPosition) {
        case 0:  // PC0
            PORT.PSRA.BIT.PSEL6 = 1;
            break;
        case 1:  // PC1
            PORT.PSRA.BIT.PSEL7 = 1;
            break;
        }
        break;
    }
}


void PortManager::setPinLevel(portName port, pinPosition pinPosition, pinLevel level) {
    if (level == Low) {
        switch (port) {
        case Port0:
            PORT0.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case Port1:
            PORT1.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case Port2:
            PORT2.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case Port3:
            PORT3.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case Port4:
            PORT4.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case PortA:
            PORTA.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case PortB:
            PORTB.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case PortC:
            PORTC.PODR.BYTE &= ~(1 << pinPosition);
            break;
        case PortE:
            PORTE.PODR.BYTE &= ~(1 << pinPosition);
            break;
        default:
            break;
        }
    } else {
        switch (port) {
        case Port0:
            PORT0.PODR.BYTE |= 1 << pinPosition;
            break;
        case Port1:
            PORT1.PODR.BYTE |= 1 << pinPosition;
            break;
        case Port2:
            PORT2.PODR.BYTE |= 1 << pinPosition;
            break;
        case Port3:
            PORT3.PODR.BYTE |= 1 << pinPosition;
            break;
        case Port4:
            PORT4.PODR.BYTE |= 1 << pinPosition;
            break;
        case PortA:
            PORTA.PODR.BYTE |= 1 << pinPosition;
            break;
        case PortB:
            PORTB.PODR.BYTE |= 1 << pinPosition;
            break;
        case PortC:
            PORTC.PODR.BYTE |= 1 << pinPosition;
            break;
        case PortE:
            PORTE.PODR.BYTE |= 1 << pinPosition;
            break;
        default:
            break;
        }
    }
}

PortManager::pinLevel PortManager::getPinLevel(portName port, pinPosition pinPosition) {
    uint16_t result;
    switch (port) {
    case Port0:
        result = (PORT0.PIDR.BYTE & (1 << pinPosition));
        break;
    case Port1:
        result = (PORT1.PIDR.BYTE & (1 << pinPosition));
        break;
    case Port2:
        result = (PORT2.PIDR.BYTE & (1 << pinPosition));
        break;
    case Port3:
        result = (PORT3.PIDR.BYTE & (1 << pinPosition));
        break;
    case Port4:
        result = (PORT4.PIDR.BYTE & (1 << pinPosition));
        break;
    case PortA:
        result = (PORTA.PIDR.BYTE & (1 << pinPosition));
        break;
    case PortB:
        result = (PORTB.PIDR.BYTE & (1 << pinPosition));
        break;
    case PortC:
        result = (PORTC.PIDR.BYTE & (1 << pinPosition));
        break;
    case PortE:
        result = (PORTE.PIDR.BYTE & (1 << pinPosition));
        break;
    default:
        break;
    }
    result = result >> pinPosition;
    return (pinLevel) result;
}

void PortManager::setPinFunctionByte(portName port, pinPosition pinPosition, uint8_t byte) {
    MPC.PWPR.BIT.B0WI = 0;
    MPC.PWPR.BIT.PFSWE = 1;
    switch (port) {
    case Port0:
        switch (pinPosition) {
        case Pin3:
            MPC.P03PFS.BYTE = byte;
            break;
        case Pin5:
            MPC.P05PFS.BYTE = byte;
            break;
        default:
            break;
        }
        break;
    case Port1:
        switch (pinPosition) {
        case Pin4:
            MPC.P14PFS.BYTE = byte;
            break;
        case Pin5:
            MPC.P15PFS.BYTE = byte;
            break;
        case Pin6:
            MPC.P16PFS.BYTE = byte;
            break;
        case Pin7:
            MPC.P17PFS.BYTE = byte;
            break;
        }
        break;
    case Port2:
        switch (pinPosition) {
        case Pin2:
            MPC.P26PFS.BYTE = byte;
            break;
        case Pin7:
            MPC.P27PFS.BYTE = byte;
            break;
        }
        break;
    case Port3:
        switch (pinPosition) {
        case Pin0:
            MPC.P30PFS.BYTE = byte;
            break;
        case Pin1:
            MPC.P31PFS.BYTE = byte;
            break;
        case Pin2:
            MPC.P32PFS.BYTE = byte;
            break;
        }
        break;
    case Port4:
        switch (pinPosition) {
        case Pin0:
            MPC.P40PFS.BYTE = byte;
            break;
        case Pin1:
            MPC.P41PFS.BYTE = byte;
            break;
        case Pin2:
            MPC.P42PFS.BYTE = byte;
            break;
        case Pin3:
            MPC.P43PFS.BYTE = byte;
            break;
        case Pin4:
            MPC.P44PFS.BYTE = byte;
            break;
        }
        break;
    case PortA:
        switch (pinPosition) {
        case Pin0:
            MPC.PA0PFS.BYTE = byte;
            break;
        case Pin1:
            MPC.PA1PFS.BYTE = byte;
            break;
        case Pin3:
            MPC.PA3PFS.BYTE = byte;
            break;
        case Pin4:
            MPC.PA4PFS.BYTE = byte;
            break;
        case Pin6:
            MPC.PA6PFS.BYTE = byte;
            break;
        }
        break;
    case PortC:
        switch (pinPosition) {
        case Pin2:
            MPC.PC2PFS.BYTE = byte;
            break;
        case Pin3:
            MPC.PC3PFS.BYTE = byte;
            break;
        case Pin4:
            MPC.PC4PFS.BYTE = byte;
            break;
        case Pin5:
            MPC.PC5PFS.BYTE = byte;
            break;
        case Pin6:
            MPC.PC6PFS.BYTE = byte;
            break;
        case Pin7:
            MPC.PC7PFS.BYTE = byte;
            break;
        }
        break;
    case PortE:
        switch (pinPosition) {
        case Pin0:
            MPC.PE0PFS.BYTE = byte;
            break;
        case Pin1:
            MPC.PE1PFS.BYTE = byte;
            break;
        case Pin2:
            MPC.PE2PFS.BYTE = byte;
            break;
        case Pin3:
            MPC.PE3PFS.BYTE = byte;
            break;
        case Pin4:
            MPC.PE4PFS.BYTE = byte;
            break;
        case Pin5:
            MPC.PE5PFS.BYTE = byte;
            break;
        case Pin6:
            MPC.PE6PFS.BYTE = byte;
            break;
        case Pin7:
            MPC.PE7PFS.BYTE = byte;
            break;
        }
        break;

    default:
        break;
    }

    MPC.PWPR.BIT.PFSWE = 0;
    MPC.PWPR.BIT.B0WI = 1;
}

}  // namespace AME_SRC

