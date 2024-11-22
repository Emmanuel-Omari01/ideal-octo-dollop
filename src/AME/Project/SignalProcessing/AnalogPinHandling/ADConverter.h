/*
 * ADConverter.h
 *
 *  Created on: 17.11.2021
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2021 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ADCONVERTER_H_
#define SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ADCONVERTER_H_

namespace AME_SRC {

#include <platform.h>

class ADConverter {
 public:
    enum scanMode {
        SingleScan = 0x00, GroupScan = 0x01, ContinuousScan = 0x10
    };
    typedef enum {
        channel0,
        channel1,
        channel2,
        channel3,
        channel4,
        channel6 = 0x6,
        channel8 = 8,
        channel9 = 9,
        channel10 = 10,
        channel11 = 11,
        channel12 = 12,
        channel13 = 13,
        channel14 = 14,
        channel15 = 15
    } scanChannel;

    void startConversion();
    void stopConversion();
    void enableConversionInGroupAForChannel(scanChannel channel);
    double getDataFromChannel(scanChannel channel);
    static ADConverter* getInstance();
    virtual ~ADConverter();

    void setConversionFinnished(bool isConversionFinnished) {
        this->conversionFinnished = conversionFinnished;
    }

    bool isConversionFinnished() {
        return conversionFinnished;
    }

    void waitForConversionCompleted();

 private:
    static bool conversionFinnished;
    static bool isNotInstanciated;
    static ADConverter *singlePtr;
    void conversionFinished();
    ADConverter();
};

}  // namespace AME_SRC

extern "C" void s12AD0_callback();

#endif  // SRC_AME_PROJECT_SIGNALPROCESSING_ANALOGPINHANDLING_ADCONVERTER_H_
