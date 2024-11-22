/*
 * SerialComInterface.cpp
 *
 *  Created on: 05.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "SerialComInterface.h"
#include <locale.h>  // For NULL definition

#include <platform.h>
#include "../../SystemBase/MicrocontrollerConfig.h"
#include "SciParam.h"
#include "../../SystemBase/MainLoop.h"

namespace AME_SRC {

uint32_t SerialComInterface::fastModeContainerLength = 0;
char *SerialComInterface::fastModeContainerPtr = 0;

SerialComInterface *SerialComInterface::sci1Ptr = 0;
SerialComInterface *SerialComInterface::sci5Ptr = 0;
SerialComInterface *SerialComInterface::sci12Ptr = 0;

bool SerialComInterface::fastMode = true;

void (*SerialComInterface::sci5TransmissionEndInterruptPtr)() = 0;

SerialComInterface::SerialComInterface(SCINumber sciNumber, Baudrate newbaud,
        SerialProfile &newProfilePtr) :
        id(sciNumber), baud(newbaud), reflectionState(false) {
    profilePtr = &newProfilePtr;
    initSCI();
}

SerialComInterface::SerialComInterface(SCINumber sciNumber) :
        baud(baud9600), reflectionState(false), profilePtr(0) {
    id = sciNumber;
}


SerialComInterface::~SerialComInterface() {
    // TODO(AME) Auto-generated destructor stub
}

void SerialComInterface::configurateRxAndTxPort() {
    switch (this->id) {
    case sci1:
        rxPort = Pin(PortManager::PortC, PortManager::Pin6, PortManager::Input,
                PortManager::PeriphalFunction);
        txPort = Pin(PortManager::PortC, PortManager::Pin7, PortManager::Output,
                PortManager::PeriphalFunction);
        setRxAndTxPinFunction(RXD1_TXD1_48PIN);
        break;
    case sci5:
        rxPort = Pin(PortManager::PortA, PortManager::Pin3, PortManager::Input,
                PortManager::PeriphalFunction);
        txPort = Pin(PortManager::PortA, PortManager::Pin4, PortManager::Output,
                PortManager::PeriphalFunction);
        setRxAndTxPinFunction(RXD5_TXD5_48PIN);
        break;
    case sci12:
        rxPort = Pin(PortManager::PortE, PortManager::Pin2, PortManager::Input,
                PortManager::PeriphalFunction);
        txPort = Pin(PortManager::PortE, PortManager::Pin1, PortManager::Output,
                PortManager::PeriphalFunction);
        setRxAndTxPinFunction(RXD12_TXD12_48PIN);
        break;
    }
}

void SerialComInterface::setRxAndTxPinFunction(pinFunction function) {
    rxPort.setFunctionByte(function);
    txPort.setFunctionByte(function);
}

void SerialComInterface::stopSCIClock() {
    MicrocontrollerConfig::disableWriteProtection();
    switch (this->id) {
    case sci1:
        MSTP_SCI1 = 0;
        break;
    case sci5:
        MSTP_SCI5 = 0;
        break;
    case sci12:
        MSTP_SCI12 = 0;
        break;
    }
    MicrocontrollerConfig::enableWriteProtection();
}

void SerialComInterface::changeBaudRate(Baudrate _baud_) {
    this->baud = _baud_;
    uint8_t baudRegValue = getBaudRegValue(_baud_);
    setTransmissionState(disabled);
    SciRegisterController::setRegisterByteBRRforSCI(baudRegValue, id);
    setTransmissionState(enabled);
}

void SerialComInterface::configurateSciInterrupts() {
    switch (this->id) {
    case sci1:
            IPR(SCI1, RXI1)= 0x7;   // Set SCI1 RXI1 interrupt priority level to 7
            IEN(SCI1, RXI1) = 0x1;  // Enable SCI1 RXI1 interrupts
            IR(SCI1, RXI1) = 0x0;   // Clear SCI1 RXI1 IR flag
            IPR(SCI1, TXI1) = 0x7;  // Set SCI1 TXI1 interrupt priority level to 7
            IEN(SCI1, TXI1) = 0x1;  // Enable SCI1 TXI1 interrupts
            IR(SCI1, TXI1) = 0x0;   // Clear SCI1 TXI1 IR flag
            break;
        case sci5:
            IPR(SCI5, RXI5) = 0x7;  // Set SCI5 RXI5 interrupt priority level to 7
            IEN(SCI5, RXI5) = 0x1;  // Enable SCI5 RXI5 interrupts
            IR(SCI5, RXI5) = 0x0;   // Clear SCI5 RXI5 IR flag
            IPR(SCI5, TXI5) = 0x7;  // Set SCI5 TXI5 interrupt priority level to 7
            IEN(SCI5, TXI5) = 0x1;  // Enable SCI5 TXI5 interrupts
            IR(SCI5, TXI5) = 0x0;   // Clear SCI5 TXI5 IR flag
            break;
        case sci12:
            IPR(SCI12, RXI12) = 0x7;    // Set SCI12 RXI12 interrupt priority level to 7
            IEN(SCI12, RXI12) = 0x1;    // Enable SCI12 RXI12 interrupts
            IR(SCI12, RXI12) = 0x0;     // Clear SCI12 RXI12 IR flag
            IPR(SCI12, TXI12) = 0x7;    // Set SCI12 TXI12 interrupt priority level to 7
            IEN(SCI12, TXI12) = 0x1;    // Enable SCI12 TXI12 interrupts
            IR(SCI12, TXI12) = 0x0;     // Clear SCI12 TXI12 IR flag
            ICU.IPR[IR_SCI12_ERI12].BYTE = 7;  // IPR(SCI12, ERI12) = 0x07;  // Set interrupt priority level to 7
            ICU.IER[0x1D].BIT.IEN6 = 0x1;      // Enable SCI12 ERI12 interrupts
            ICU.IR[IR_SCI12_ERI12].BIT.IR = 0;  // IR(SCI12, ERI12) = 0;    // Clear SCI12 ERI12 IR flag
            break;
    }
}

void SerialComInterface::enableInterrupts() {
    configurateSciInterrupts();
}

void SerialComInterface::disableInterrupts() {
    switch (this->id) {
    case sci1:
            IEN(SCI1, RXI1) = 0x0;  // Disable SCI1, RXI1 interrupts
            IEN(SCI1, TXI1) = 0x0;  // Disable SCI1 TXI1 interrupts
            IR(SCI1, RXI1) = 0x0;   // Clear SCI1 RXI1 IR flag
            IR(SCI1, TXI1) = 0x0;   // Clear SCI1 TXI1 IR flag
            break;
        case sci5:
            IEN(SCI5, RXI5) = 0x0;  // Disable SCI5 RXI5 interrupts
            IEN(SCI5, TXI5) = 0x0;  // Disable SCI5 TXI5 interrupts
            ICU.IER[0x1B].BIT.IEN6 = 0x0;  // Disable SCI5 ERI5 interrupts
            IR(SCI5, RXI5) = 0x0;   // Clear SCI5 RXI5 IR flag
            IR(SCI5, TXI5) = 0x0;   // Clear SCI5 TXI5 IR flag
            ICU.IR[IR_SCI5_ERI5].BIT.IR = 0;  // IR(SCI5, ERI5) = 0;    // Clear SCI5 ERI5 IR flag
            break;
        case sci12:
            IEN(SCI12, RXI12) = 0x0;    // Disable SCI12 RXI12 interrupts
            IEN(SCI12, TXI12) = 0x0;    // Disable SCI12 TXI12 interrupts
            ICU.IER[0x1D].BIT.IEN6 = 0x0;  // Disable SCI12 ERI12 interrupts
            IR(SCI12, RXI12) = 0x0;   // Clear SCI12 RXI12 IR flag
            IR(SCI12, TXI12) = 0x0;   // Clear SCI12 TXI12 IR flag
            ICU.IR[IR_SCI12_ERI12].BIT.IR = 0;  // IR(SCI12, ERI12) = 0;    // Clear SCI12 ERI12 IR flag
            break;
    }
}


void SerialComInterface::setTransmissionState(state _state_) {
    enum { TeReBitOffset = 4, BitCombination = 3 };
    uint8_t scrByte = SciRegisterController::getRegisterByteSCRforSCI(id);
    if (_state_ == enabled) {
        scrByte |= BitCombination << TeReBitOffset;
    } else {
        scrByte &= (uint8_t) (~(BitCombination << TeReBitOffset));
    }
    SciRegisterController::setRegisterByteSCRforSCI(scrByte, id);
}

void SerialComInterface::configurateClockSelection() {
    enum { LogicalANDoutput = 0x1, baseClockCycle8 = 0x10 };
    uint8_t targetByte = LogicalANDoutput + baseClockCycle8;
    SciRegisterController::setRegisterByteSEMRforSCI(targetByte, id);
}

void SerialComInterface::disableOtherCommunicationModes() {
    enum { DisableSmartCard = 0x72 };
    SciRegisterController::setRegisterByteSPMRforSCI(0, id);
    SciRegisterController::setRegisterByteSMRforSCI(0, id);
    SciRegisterController::setRegisterByteSCMRforSCI(DisableSmartCard, id);
}

void SerialComInterface::pushRXD() {
    char receivedLetter;
    bool dataHasBeenReceived = SciRegisterController::getRegisterByteSSRforSCI(id) & 0x80;
    if (dataHasBeenReceived) {
        receivedLetter = SciRegisterController::getRegisterByteRDRforSCI(id);
        if (isReflection()) {
            pushTXD(receivedLetter);
        }
        profilePtr->interpretReceivedLetter(receivedLetter);
    }
    receiveErrorHandling();
}

SerialComInterface *SerialComInterface::getInstance(SCINumber sciNumber) {
    switch (sciNumber) {
    case sci1:
        return sci1Ptr;
        break;
    case sci5:
        return sci5Ptr;
        break;
    case sci12:
        return sci12Ptr;
        break;
    }
    return NULL;
}

SerialComInterface *SerialComInterface::getInstance(SCINumber sciNumber,
        Baudrate newbaud, SerialProfile &newProfile) {
    switch (sciNumber) {
    case sci1:
        if (sci1Ptr == NULL) {
            sci1Ptr = new SerialComInterface(sciNumber, newbaud, newProfile);
        }
        return sci1Ptr;
        break;
    case sci5:
        if (sci5Ptr == NULL) {
            sci5Ptr = new SerialComInterface(sciNumber, newbaud, newProfile);
        }
        return sci5Ptr;
        break;
    case sci12:
        if (sci12Ptr == NULL) {
            sci12Ptr = new SerialComInterface(sciNumber, newbaud, newProfile);
        }
        return sci12Ptr;
        break;
    }
    return NULL;
}

// void SerialComInterface::prepareInstanceForSciObject(const SerialComInterface &sciPtr,
//          Baudrate newbaud, SerialProfile &newProfile) {
//      sciPtr.buffer.indexIN = 0;
//      sciPtr.buffer.indexOut = 0;
//      sciPtr.baud = newbaud;
//      sciPtr.initSCI();
//      sciPtr.setProfilePtr(&newProfile);
// }

int SerialComInterface::txdFree() {
    int result = kBufferSize - 1, local_ptr = buffer.indexOut;

    while (local_ptr != buffer.indexIN) {
        result--;
        if (++local_ptr >= kBufferSize)  // handle buffer overflow
            local_ptr = 0;
    }
    return result;
}

// baseClockCycle8 => SEMR.abcs = 1
uint8_t SerialComInterface::getBaudRegValue(Baudrate _baud_) {
    uint8_t result;
    // Error look at Hardware Manual page 759 26.2.9 brr
    switch (_baud_) {
    case baud9600:
        result = 155;
        break;
    case baud115200:  // Error => ((48*10^6 / 47923200) -1) *100 = 16%
        result = 12;
        break;
    case baud230400:  // Error => 8 %
        result = 5;
        break;
    case baud300000:  // Error => ((48*10^6 / 48*10^6) -1) *100 = 0%
        result = 4;
        break;
    case baud375000:  // Error => ((48*10^6 / 48*10^6) -1) *100 = 0%
        result = 3;
        break;
    case baud500000:
        result = 2;
        break;
    case baud750000:
        result = 1;
        break;
    default:
        result = 12;
        break;
    }
    return result;
}

void SerialComInterface::initSCI() {
    configurateRxAndTxPort();
    stopSCIClock();
    configurateSciInterrupts();
    configurateClockSelection();
    disableOtherCommunicationModes();
    changeBaudRate(baud);
    setReceiveTransmitInterruptStatus(enabled);
}

void SerialComInterface::setReceiveTransmitInterruptStatus(state irqState) {
    if (irqState == enabled) {
        enum { TieRieBitOffset = 6, BitCombination = 3 };
        uint8_t scrByte = SciRegisterController::getRegisterByteSCRforSCI(id);
        if (irqState == enabled) {
            scrByte |= BitCombination << TieRieBitOffset;
        } else {
            scrByte &= (uint8_t) (~(BitCombination << TieRieBitOffset));
        }
        SciRegisterController::setRegisterByteSCRforSCI(scrByte, id);
    }
}


void SerialComInterface::receiveErrorHandling() {
    uint8_t errorControlByte = SciRegisterController::getRegisterByteSSRforSCI(id);
    bool isErrorOccurred = errorControlByte & 0x38;
    if (isErrorOccurred) {  // ORER=Overflow, FER=Frame, PER=Parity Error?
        errorControlByte &= (uint8_t) (~0x38);  // clear error flag to allow receive again
        SciRegisterController::setRegisterByteSSRforSCI(errorControlByte, id);
        if (isReflection()) {
            pushTXD('?');
        }
    }
}

void SerialComInterface::pushTXD(char outPutLetter) {
    buffer.container[buffer.indexIN] = outPutLetter;
    if (++buffer.indexIN >= kBufferSize) {
        buffer.indexIN = 0;
    }
}

void SerialComInterface::popTXD() {
    char outPutLetter;
    bool isTransmissionBufferEmpty = SciRegisterController::getRegisterByteSSRforSCI(id) & 0x80;
    if (isTransmissionBufferEmpty) {
        // handle user outputs
        if (buffer.indexIN != buffer.indexOut) {
            outPutLetter = buffer.container[buffer.indexOut];
            SciRegisterController::setRegisterByteTDRforSCI(outPutLetter, id);  // output
            if (++buffer.indexOut >= kBufferSize) {
                buffer.indexOut = 0;
            }
        }
    }
}

bool SerialComInterface::isTransmissionComplete() {
    return buffer.indexIN == buffer.indexOut;
}


// void pushRXD12(){
//      char receivedLetter;
//      bool dataHasBeenReceived = SciRegisterController::getRegisterByteSSRforSCI12() & 0x80;
//      if (dataHasBeenReceived) {
//          receivedLetter = SciRegisterController::getRegisterByteRDRforSCI12();
//          if(SerialComInterface::sci12Object.isReflection()) {
//              //pushTXD(receivedLetter);
//          }
//          SerialComInterface::sci12Object.profilePtr->interpretReceivedLetter(receivedLetter);
//      }
//      // receiveErrorHandling();
//      bool isErrorOccurred = SciRegisterController::getRegisterByteSSRforSCI12() & 0x38;
//      if (isErrorOccurred) {      // ORER=Overflow, FER=Frame, PER=Parity Error?
//          uint8_t errorControlByte = SciRegisterController::getRegisterByteSSRforSCI12() ;
//          errorControlByte &= ~ 0x38;     // clear error flag to allow receive again
//          SciRegisterController::setRegisterByteSSRforSCI(errorControlByte, sci12);
//          if (SerialComInterface::sci12Object.isReflection()){
//              // pushTXD('?');
//          }
//      }
//  }
//
// void pushRXD5() {
//      char receivedLetter;
//      bool dataHasBeenReceived = SciRegisterController::getRegisterByteSSRforSCI5() & 0x80 ;
//      if (dataHasBeenReceived) {
//          receivedLetter = SciRegisterController::getRegisterByteRDRforSCI5();
//          if(SerialComInterface::sci5Object.isReflection()) {
//             // pushTXD(receivedLetter);
//          }
//          SerialComInterface::sci5Object.profilePtr->interpretReceivedLetter(receivedLetter);
//      }
//      // receiveErrorHandling();
//      bool isErrorOccurred = SciRegisterController::getRegisterByteSSRforSCI5() & 0x38;
//      if (isErrorOccurred) {  // ORER=Overflow, FER=Frame, PER=Parity Error?
//          uint8_t errorControlByte = SciRegisterController::getRegisterByteSSRforSCI5();
//          errorControlByte &= ~ 0x38;     // clear error flag to allow receive again
//          SciRegisterController::setRegisterByteSSRforSCI(errorControlByte, sci5);
//          if (SerialComInterface::sci5Object.isReflection()){
//              // pushTXD('?');
//          }
//      }
// }
//
// void popTXD12() {
//      char outPutLetter;
//      bool isTransmissionBufferEmpty = SciRegisterController::getRegisterByteSSRforSCI12() & 0x80;
//      if (isTransmissionBufferEmpty) {
//          // handle user outputs
//          if (SerialComInterface::sci12Object.buffer.indexIN != SerialComInterface::sci12Object.buffer.indexOut) {
//              outPutLetter = SerialComInterface::sci12Object.buffer.
//                                  container[SerialComInterface::sci12Object.buffer.indexOut];
//              SciRegisterController::setRegisterByteTDRforSCI12(outPutLetter);
//              if (++SerialComInterface::sci12Object.buffer.indexOut >= SerialComInterface::sci12Object.bufferSize) {
//                  SerialComInterface::sci12Object.buffer.indexOut=0;
//              }
//          }
//      }
// }
//
// void popTXD5() {
//      char outPutLetter;
//      bool isTransmissionBufferEmpty = SciRegisterController::getRegisterByteSSRforSCI5() & 0x80;
//      if (isTransmissionBufferEmpty) {
//          // handle user outputs
//          if (SerialComInterface::sci5Object.buffer.indexIN != SerialComInterface::sci5Object.buffer.indexOut) {
//              if((SerialComInterface::sci5Object.buffer.indexIN-1) ==
//                      SerialComInterface::sci5Object.buffer.indexOut) {
//                  if(SerialComInterface::sci5Object.sci5TransmissionEndInterruptPtr != 0) {
//                      SerialComInterface::sci5Object.sci5TransmissionEndInterruptPtr();
//                  }
//              }
//              outPutLetter = SerialComInterface::sci5Object.buffer.
//                              container[SerialComInterface::sci5Object.buffer.indexOut];
//              SciRegisterController::setRegisterByteTDRforSCI5(outPutLetter);
//              if (++SerialComInterface::sci5Object.buffer.indexOut >= SerialComInterface::sci5Object.bufferSize) {
//                  SerialComInterface::sci5Object.buffer.indexOut=0;
//              }
//          }
//      }
// }

void popTXDfast() {
    char outPutLetter;
    bool isTransmissionBufferEmpty =
            SciRegisterController::getRegisterByteSSRforSCI5() & 0x80;
    static uint32_t counter = 0;
    if (isTransmissionBufferEmpty) {
        // handle user outputs
        if ((counter < SerialComInterface::sci5Ptr->fastModeContainerLength)
                && (SerialComInterface::sci5Ptr->fastModeContainerPtr != 0)) {
            outPutLetter = SerialComInterface::sci5Ptr->fastModeContainerPtr[counter++];
            SciRegisterController::setRegisterByteTDRforSCI5(outPutLetter);     // output
        } else {
            counter = 0;
            if (SerialComInterface::sci5Ptr->sci5TransmissionEndInterruptPtr != 0) {
                SerialComInterface::sci5Ptr->sci5TransmissionEndInterruptPtr();
            }
        }
    }
}

}  // namespace AME_SRC

//------------------------------------------------------------------------------
void rxd12_int(void) {
    // AME_SRC::pushRXD12();
    AME_SRC::SerialComInterface::getInstance(AME_SRC::sci12)->pushRXD();
}
//------------------------------------------------------------------------------
void txd12_int(void) {
    // AME_SRC::popTXD12();
    AME_SRC::SerialComInterface::getInstance(AME_SRC::sci12)->popTXD();
}
//------------------------------------------------------------------------------
void eri12_int(void) {
    AME_SRC::SerialComInterface::getInstance(AME_SRC::sci12)->receiveErrorHandling();
}

//------------------------------------------------------------------------------
void rxd1_int(void) {
    AME_SRC::SerialComInterface::getInstance(AME_SRC::sci1)->pushRXD();
}
//------------------------------------------------------------------------------
void txd1_int(void) {
    AME_SRC::SerialComInterface::getInstance(AME_SRC::sci1)->popTXD();
}
//------------------------------------------------------------------------------
void eri5_int(void) {
    AME_SRC::SerialComInterface::getInstance(AME_SRC::sci5)->receiveErrorHandling();
}
//------------------------------------------------------------------------------
void rxd5_int(void) {
    // AME_SRC::pushRXD5();
    AME_SRC::SerialComInterface::getInstance(AME_SRC::sci5)->pushRXD();
}
//------------------------------------------------------------------------------
void txd5_int(void) {
    if (AME_SRC::SerialComInterface::isFastMode()) {
        AME_SRC::popTXDfast();
        // AME_SRC::SerialComInterface::getInstance(AME_SRC::sci5)->popTXDfast();
    } else {
        // AME_SRC::popTXD5();
        AME_SRC::SerialComInterface::getInstance(AME_SRC::sci5)->popTXD();
    }
}

