/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include "PCBPortSelector.h"

#include <cstddef>

namespace AME_SRC {

IoPin *PCBPortSelector::onboardLED1 = NULL;
IoPin *PCBPortSelector::onboardLED2 = NULL;
IoPin *PCBPortSelector::onboardLED3 = NULL;

IoPin *PCBPortSelector::channel = NULL;
IoPin *PCBPortSelector::mux0 = NULL;
IoPin *PCBPortSelector::mux1 = NULL;
IoPin *PCBPortSelector::mux2 = NULL;
Multiplexor* PCBPortSelector::multiplexor = NULL;
MuxIoPin *PCBPortSelector::mux_in1 = NULL;
MuxIoPin *PCBPortSelector::mux_in2 = NULL;
MuxIoPin *PCBPortSelector::mux_in3 = NULL;
MuxIoPin *PCBPortSelector::mux_in4 = NULL;

IoPin *PCBPortSelector::in1 = NULL;
IoPin *PCBPortSelector::in2 = NULL;
IoPin *PCBPortSelector::in3 = NULL;
IoPin *PCBPortSelector::in4 = NULL;
IoPin *PCBPortSelector::CP3_Boot0 = NULL;
IoPin *PCBPortSelector::FCC_CENELEC_A1 = NULL;
IoPin *PCBPortSelector::FCC_CENELEC_A2 = NULL;
IoPin *PCBPortSelector::BackUp = NULL;
IoPin *PCBPortSelector::cpxResetPin = NULL;
IoPin *PCBPortSelector::FCC_CENELEC_IN = NULL;

IoPinGroup *PCBPortSelector::moduleInputs = NULL;
IoPinGroup *PCBPortSelector::moduleOutputs = NULL;

IoPin *PCBPortSelector::out1 = NULL;
IoPin *PCBPortSelector::out2 = NULL;
IoPin *PCBPortSelector::out3 = NULL;
IoPin *PCBPortSelector::out4 = NULL;

AnalogPin *PCBPortSelector::cpu_Analog1 = NULL;
AnalogPin *PCBPortSelector::cpu_Analog2 = NULL;
AnalogPin *PCBPortSelector::cpu_Analog3 = NULL;
AnalogPin *PCBPortSelector::cpu_Analog4 = NULL;
AnalogPin *PCBPortSelector::cpu_Analog5 = NULL;

IoPin PCBPortSelector::nullPin = IoPin();
IoPinGroup PCBPortSelector::nullGroup = IoPinGroup();
AnalogPin PCBPortSelector::nullAnalogPin = AnalogPin();

uint8_t PCBPortSelector::myVersion;

PCBPortSelector::PCBPortSelector() = default;

#pragma GCC diagnostic ignored "-Wstack-usage="
void PCBPortSelector::init(eBaseVersion version) {
    myVersion = (uint8_t) version;
    switch (version) {
    case Evaluation:
        onboardLED1 = new IoPin(PortManager::Port1, PortManager::Pin7, PortManager::Output, Pin::active);  // P17 (PB5)
        onboardLED2 = new IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output, Pin::active);  // PA1

        // Multiplexor
        mux0 = new IoPin(PortManager::PortC, PortManager::Pin5, PortManager::Output);          // PC5
        mux1 = new IoPin(PortManager::PortC, PortManager::Pin6, PortManager::Output);          // PC6
        mux2 = new IoPin(PortManager::PortC, PortManager::Pin7, PortManager::Output);          // PC7
        channel = new IoPin(PortManager::Port3, PortManager::Pin5, PortManager::Input);        // P35

        multiplexor = new Multiplexor(mux0, mux1, mux2, channel);
        mux_in1 = new MuxIoPin(multiplexor, 4);
        mux_in2 = new MuxIoPin(multiplexor, 2);
        mux_in3 = new MuxIoPin(multiplexor, 6);
        mux_in4 = new MuxIoPin(multiplexor, 1);
        // MuxIoPin mux_in5 = MuxIoPin(multiplexor, 7);
        // MuxIoPin mux_in6 = MuxIoPin(multiplexor, 5);
        // MuxIoPin mux_in7 = MuxIoPin(multiplexor, 0);
        // MuxIoPin mux_in8 = MuxIoPin(multiplexor, 3);

        moduleInputs = new IoPinGroup(4, mux_in1, mux_in2, mux_in3, mux_in4);

        cpxResetPin = new IoPin(PortManager::Port2, PortManager::Pin6, PortManager::Output);  // P26

        out1 = new IoPin(PortManager::PortE, PortManager::Pin0, PortManager::Output);      // PE0
        out2 = new IoPin(PortManager::PortE, PortManager::Pin7, PortManager::Output);      // PE7
        out3 = new IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output);      // PA1
        out4 = new IoPin(PortManager::PortE, PortManager::Pin3, PortManager::Output);      // PE3
        moduleOutputs = new IoPinGroup(4, out1, out2, out3, out4);

        cpu_Analog1 = new AnalogPin(PortManager::Port4, PortManager::Pin6, PortManager::Input);  // P46
        cpu_Analog2 = new AnalogPin(PortManager::Port4, PortManager::Pin2, PortManager::Input);  // P42
        cpu_Analog3 = new AnalogPin(PortManager::Port4, PortManager::Pin1, PortManager::Input);  // P41
        cpu_Analog4 = new AnalogPin(PortManager::Port4, PortManager::Pin0, PortManager::Input);  // P40
        break;
    case Version1_1:
        onboardLED1 = new IoPin(PortManager::Port0, PortManager::Pin3, PortManager::Output, Pin::active);  // P03
        onboardLED2 = new IoPin(PortManager::Port4, PortManager::Pin1, PortManager::Output, Pin::active);  // P41
        onboardLED3 = new IoPin(PortManager::Port4, PortManager::Pin2, PortManager::Output, Pin::active);  // P42

        CP3_Boot0 =     new IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output);   // PA1
        FCC_CENELEC_A1 = new IoPin(PortManager::PortC, PortManager::Pin0, PortManager::Output);   // PC0
        FCC_CENELEC_A2 = new IoPin(PortManager::PortC, PortManager::Pin1, PortManager::Output);   // PC1
        BackUp =        new IoPin(PortManager::PortE, PortManager::Pin3, PortManager::Output);   // PE3

        cpxResetPin = new IoPin(PortManager::PortA, PortManager::Pin0, PortManager::Output);  // PA0

        out1 = new IoPin(PortManager::PortE, PortManager::Pin7, PortManager::Output);   // PE7 motor 1 Relay
        out2 = new IoPin(PortManager::PortE, PortManager::Pin0, PortManager::Output);   // PE0 motor 2 Relay
        moduleOutputs = new IoPinGroup(2, out1, out2);

        cpu_Analog1 = new AnalogPin(PortManager::Port4, PortManager::Pin0, PortManager::Input);  // P40
        break;
    case Version1_2:
        onboardLED1 = new IoPin(PortManager::PortC, PortManager::Pin1, PortManager::Output, Pin::active);  // PC1
        onboardLED2 = new IoPin(PortManager::PortC, PortManager::Pin0, PortManager::Output, Pin::active);  // PC0
        onboardLED3 = new IoPin(PortManager::PortB, PortManager::Pin5, PortManager::Output, Pin::active);  // PB5

        in1 = new IoPin(PortManager::PortE, PortManager::Pin6, PortManager::Input);  // PE6 Input E1.1
        in2 = new IoPin(PortManager::Port4, PortManager::Pin4, PortManager::Input);  // P44 Input E1.2
        in3 = new IoPin(PortManager::PortE, PortManager::Pin7, PortManager::Input);  // PE7 Input E2.1
        in4 = new IoPin(PortManager::Port4, PortManager::Pin6, PortManager::Input);  // P46 Input E2.2
        moduleInputs = new IoPinGroup(4, in1, in2, in3, in4);

        CP3_Boot0 = new IoPin(PortManager::PortA, PortManager::Pin1, PortManager::Output);    // PA1
        cpxResetPin = new IoPin(PortManager::PortE, PortManager::Pin3, PortManager::Output);  // PE3

        FCC_CENELEC_IN = new IoPin(PortManager::PortC, PortManager::Pin3, PortManager::Input);    // PC3
                            // ToDo(AME) use FCC_CENELEC_IN input for modem init. in RequestFactory

        out1 = new IoPin(PortManager::Port0, PortManager::Pin3, PortManager::Output);   // P03 motor 1 Relay
        out2 = new IoPin(PortManager::Port0, PortManager::Pin5, PortManager::Output);   // P05 motor 2 Relay
        moduleOutputs = new IoPinGroup(2, out1, out2);

        cpu_Analog1 = new AnalogPin(PortManager::Port4, PortManager::Pin1, PortManager::Input);  // Version_A1, P41
        cpu_Analog2 = new AnalogPin(PortManager::Port4, PortManager::Pin2, PortManager::Input);  // Version_A2, P42
        cpu_Analog3 = new AnalogPin(PortManager::Port4, PortManager::Pin3, PortManager::Input);  // Version_B1, P43
        cpu_Analog4 = new AnalogPin(PortManager::PortE, PortManager::Pin4, PortManager::Input);  // Version_B2, PE4
        cpu_Analog5 = new AnalogPin(PortManager::PortE, PortManager::Pin5, PortManager::Input);  // Voltage_Vline PE5
        break;
    }
}

IoPin* PCBPortSelector::getPin(ePinGroup pg, uint32_t index) {
    switch (pg) {
    case LED:
        switch (index) {
        case 1: return onboardLED1;
        case 2: return onboardLED2;
        case 3: return onboardLED3;
        }
        break;
    case MultiplexorItem:
        // ToDo(AME): convert magic numbers if multiplexor is needed
        switch (index) {
        case  1: return mux_in1;
        case  2: return mux_in2;
        case  3: return mux_in3;
        case  4: return mux_in4;
        case 10: return channel;
        case 11: return mux0;
        case 12: return mux1;
        case 13: return mux2;
        }
        break;
    case ModuleInputPin:
        switch (index) {
        case 1: return in1;
        case 2: return in2;
        case 3: return in3;
        case 4: return in4;
        }
        break;
    case ModuleOutputPin:
        switch (index) {
        case 1: return out1;
        case 2: return out2;
        case 3: return out3;
        case 4: return out4;
        }
        break;
    case CPXResetPin:
        return cpxResetPin;
    case CPX3Boot:
        return CP3_Boot0;
    case FCC_Cenelec_Out:
        switch (index) {
        case 1: return FCC_CENELEC_A1;
        case 2: return FCC_CENELEC_A2;
        }
        break;
    case FCC_Cenelec_In:
        return FCC_CENELEC_IN;
    case CPXBackup:
        return BackUp;
    }
    return &nullPin;
}

bool PCBPortSelector::isValidPin(IoPin *checkPin) {
    return checkPin != NULL && checkPin != &nullPin;
}


IoPinGroup* PCBPortSelector::getGroup(ePinGroup pg) {
    switch (pg) {
    case ModuleInGroup:
        return moduleInputs;
    case ModuleOutGroup:
        return moduleOutputs;
    }
    return &nullGroup;
}


bool PCBPortSelector::isValidGroup(IoPinGroup *checkGroup) {
    return checkGroup != NULL && checkGroup != &nullGroup;
}


Multiplexor * PCBPortSelector::getMultiplexor() {
    return multiplexor;
}

AnalogPin* PCBPortSelector::getAnalogPin(uint32_t index) {
    switch (index) {
    case 1: return cpu_Analog1;
    case 2: return cpu_Analog2;
    case 3: return cpu_Analog3;
    case 4: return cpu_Analog4;
    case 5: return cpu_Analog5;
    }
    return &nullAnalogPin;
}

bool PCBPortSelector::isValidAnalogPin(AnalogPin *checkPin) {
    return checkPin != NULL && checkPin != &nullAnalogPin;
}

char* PCBPortSelector::versionToString(char *output, uint32_t maxLen) {
    switch (myVersion) {
    case Evaluation:
        snprintf(output, maxLen, TX::getText(TX::dTxtHardwareVersion), "1.0 Prototype / Evaluation");
        break;
    case Version1_1:
        snprintf(output, maxLen, TX::getText(TX::dTxtHardwareVersion), "1.1");
        break;
    case Version1_2:
        snprintf(output, maxLen, TX::getText(TX::dTxtHardwareVersion), "1.2");
        break;
    default:
        snprintf(output, maxLen, TX::getText(TX::dTxtHardwareVersion), TX::getText(TX::txtUnknown));
        break;
    }
  return output;
}


char *PCBPortSelector::analogToString(char *output, uint32_t maxLen, uint32_t index) {
    AnalogPin *myPin = getAnalogPin(index);
    if (PCBPortSelector::isValidAnalogPin(myPin)) {
        double pinCurrent = (myPin->read() / static_cast<double>(4096)) * 3.3;

        snprintf(output, maxLen, TX::getText(TX::cTxtAnalogPinMsg), index, pinCurrent);
    } else {
        snprintf(output, maxLen, TX::getText(TX::HardwareReferenceError));
    }
    return output;
}


PCBPortSelector::~PCBPortSelector() = default;

}  // namespace AME_SRC
