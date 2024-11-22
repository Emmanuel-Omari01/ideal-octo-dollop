/*
 * TimeMeasurement.h
 *
 *  Created on: 02.02.2023
 *    Modified: 09.02.2023
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_TIMEMEASUREMENT_H_
#define SRC_AME_PROJECT_SYSTEMBASE_TIMEMEASUREMENT_H_

#include "Global2.h"

namespace AME_SRC {

class TimeMeasurement {
 public:
    struct measureJob {
        bool registered;
        uint32_t time;
    };
    typedef enum {
        Raw_Millisecs, Seconds, Secs_with_ms, TimeHHMMSS, TimeHHMMSSms
    } format2str;
    enum {
        job_max = 5
    };  // const uint8_t job_max = 5;
    TimeMeasurement();
    virtual ~TimeMeasurement();
    static void start(void);
    static void restart(void);
    static void stop(void);
    static measureJob& registerTimedJob(int8_t job_next);
    static void unregisterTimedJob(int8_t jobId);
    static void unregisterTimedJob(measureJob &job);
    static uint32_t getJobDeltat(uint8_t job_nr);
    static uint32_t getJobDeltat(measureJob &job);
    static void resetJobTime(measureJob &job);
    static int getJobIndex(measureJob &job);
    static char* toString(uint8_t job_nr, char *obuf, uint32_t oLen, format2str nFormat);

 private:
    static CompareMatchTimeDriver *pCmtTimeRef;
    static void IRQTimer();
    static measureJob& GetNextJob(void);
    static uint32_t time_1ms;
    static measureJob jobs[job_max];
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_TIMEMEASUREMENT_H_
