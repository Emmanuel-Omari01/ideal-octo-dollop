/*
 * SerialComInterface.h
 *
 *  Created on: 05.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SCI_SERIALCOMINTERFACE_H_
#define SRC_AME_PROJECT_TERMINAL_SCI_SERIALCOMINTERFACE_H_

#include "SciRegisterController.h"
#include "../../SignalProcessing/IoPinHandling/Pin.h"
#include "../SerialProfiles/SerialProfile.h"

namespace AME_SRC {

class SerialComInterface {
 public:
    void changeBaudRate(Baudrate baud);

    virtual ~SerialComInterface();

    bool isReflection() const {
        return reflectionState;
    }

    void setReflectionState(bool _reflectionState_) {
        this->reflectionState = _reflectionState_;
    }

    static uint32_t fastModeContainerLength;
    static char *fastModeContainerPtr;
    static const uint16_t kBufferSize = 1024;
    static void (*sci5TransmissionEndInterruptPtr)();
    static SerialComInterface* getInstance(SCINumber sciNumber);
    static SerialComInterface* getInstance(SCINumber sciNumber, Baudrate newbaud, SerialProfile &newProfile);
    void pushRXD();
    void popTXD();
    int txdFree();
    bool isTransmissionComplete();
    void pushTXD(char outPutLetter);
    void receiveErrorHandling();
    void enableInterrupts();
    void disableInterrupts();

    Baudrate getBaud() const {
        return baud;
    }

    SerialProfile* getProfilePtr() {
        return profilePtr;
    }

    void setProfilePtr(SerialProfile *newprofilePtr) {
        profilePtr = newprofilePtr;
    }

    friend void pushRXD12();
    friend void pushRXD5();
    friend void popTXD12();
    friend void popTXD5();
    friend void popTXDfast();
    friend void eri5_int();
    friend void eri12_int();

    static bool isFastMode() {
        return fastMode;
    }

    static void setFastMode(bool status) {
        fastMode = status;
    }
    static SerialComInterface *sci5Ptr;

 private:
    static bool fastMode;
    static SerialComInterface *sci1Ptr;

    static SerialComInterface *sci12Ptr;

    SerialComInterface(SCINumber sciNumber, Baudrate newbaud, SerialProfile &newProfilePtr);
    explicit SerialComInterface(SCINumber sciNumber);

    typedef enum {
        RXD1_TXD1_48PIN = 0xB, RXD5_TXD5_48PIN = 0xA, RXD12_TXD12_48PIN = 0xC
    } pinFunction;
    typedef enum {
        disabled, enabled
    } state;

    SCINumber id;
    Pin rxPort;
    Pin txPort;
    Baudrate baud;
    bool reflectionState;
    SerialProfile *profilePtr;

    struct {
        uint16_t indexIN;
        uint16_t indexOut;
        char container[kBufferSize];
    } buffer;

    void initSCI();
    uint8_t getBaudRegValue(Baudrate baud);
    void configurateRxAndTxPort();
    void setRxAndTxPinFunction(pinFunction function);
    void stopSCIClock();
    void configurateSciInterrupts();
    void setTransmissionState(state state);
    void setReceiveTransmitInterruptStatus(state irqState);
    void configurateClockSelection();
    void disableOtherCommunicationModes();
    // void receiveErrorHandling();
    // static void prepareInstanceForSciObject(const SerialComInterface &sciPtr, Baudrate newbaud,
    //                  SerialProfile &newProfile);
};

}  // namespace AME_SRC

// Routing of the Interrupts
extern "C" void rxd12_int();
extern "C" void txd12_int();
extern "C" void eri12_int();
extern "C" void rxd1_int();
extern "C" void txd1_int();
extern "C" void eri5_int();
extern "C" void rxd5_int();
extern "C" void txd5_int();
extern void popTXDfast();

#endif  // SRC_AME_PROJECT_TERMINAL_SCI_SERIALCOMINTERFACE_H_
