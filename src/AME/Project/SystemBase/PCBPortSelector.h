/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_PCBPORTSELECTOR_H_
#define SRC_AME_PROJECT_SYSTEMBASE_PCBPORTSELECTOR_H_

#include <stdint.h>
#include "Global2.h"
#include "../SignalProcessing/IoPinHandling/IoPin.h"
#include "../SignalProcessing/IoPinHandling/IoPinGroup.h"
#include "../SignalProcessing/AnalogPinHandling/AnalogPin.h"
#include "../SignalProcessing/Sensorik/Multiplexor.h"
#include "../SignalProcessing/Sensorik/MuxIoPin.h"
#include "../SystemLogic/I18N.h"

namespace AME_SRC {

class PCBPortSelector {
 public:
    PCBPortSelector();
    virtual ~PCBPortSelector();

    enum eBaseVersion {
        Evaluation,
        Version1_1,
        Version1_2,
    };
    enum ePinGroup {
        // IoPin
        LED, MultiplexorItem,
        CPXResetPin,
        CPX3Boot,
        FCC_Cenelec_Out,
        FCC_Cenelec_In,
        CPXBackup,
        ModuleInputPin,
        ModuleOutputPin,
        // IoPinGroup
        ModuleInGroup, ModuleOutGroup,
    };
    static void init(eBaseVersion version);
    static IoPin* getPin(ePinGroup pg, uint32_t index = 0);
    static IoPinGroup* getGroup(ePinGroup pg);
    static Multiplexor * getMultiplexor();
    static AnalogPin* getAnalogPin(uint32_t index);
    static bool isValidPin(IoPin *checkPin);
    static bool isValidGroup(IoPinGroup *checkGroup);
    static bool isValidAnalogPin(AnalogPin *checkPin);
    static char* versionToString(char *output, uint32_t maxLen);
    static char *analogToString(char *output, uint32_t maxLen, uint32_t index);

 private:
    static IoPin* onboardLED1;
    static IoPin* onboardLED2;
    static IoPin* onboardLED3;

    static IoPin* channel;
    static IoPin* mux0;
    static IoPin* mux1;
    static IoPin* mux2;
    static Multiplexor* multiplexor;
    static MuxIoPin* mux_in1;
    static MuxIoPin* mux_in2;
    static MuxIoPin* mux_in3;
    static MuxIoPin* mux_in4;

    static IoPin* in1;
    static IoPin* in2;
    static IoPin* in3;
    static IoPin* in4;
    static IoPin* CP3_Boot0;
    static IoPin* FCC_CENELEC_A1;
    static IoPin* FCC_CENELEC_A2;
    static IoPin* BackUp;
    static IoPin* cpxResetPin;
    static IoPin* FCC_CENELEC_IN;

    static IoPinGroup* moduleInputs;
    static IoPinGroup* moduleOutputs;

    static IoPin* out1;
    static IoPin* out2;
    static IoPin* out3;
    static IoPin* out4;

    static AnalogPin* cpu_Analog1;
    static AnalogPin* cpu_Analog2;
    static AnalogPin* cpu_Analog3;
    static AnalogPin* cpu_Analog4;
    static AnalogPin* cpu_Analog5;

    static IoPin nullPin;
    static IoPinGroup nullGroup;
    static AnalogPin nullAnalogPin;

    static uint8_t myVersion;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_PCBPORTSELECTOR_H_
