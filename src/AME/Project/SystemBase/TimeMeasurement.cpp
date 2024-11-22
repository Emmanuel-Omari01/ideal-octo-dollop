/*
 * TimeMeasurement.cpp
 *
 *  Created on: 02.02.2023
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#include "TimeMeasurement.h"

#include <cstdio>
#include <cstring>

namespace AME_SRC {

CompareMatchTimeDriver* TimeMeasurement::pCmtTimeRef = Global2::getCmt1();
uint32_t TimeMeasurement::time_1ms = 0;
TimeMeasurement::measureJob TimeMeasurement::jobs[] = {};

TimeMeasurement::TimeMeasurement() {
    // please call start() or restart(), if the timer should start working
}

TimeMeasurement::~TimeMeasurement() = default;

void TimeMeasurement::start() {
    time_1ms = 0;
    pCmtTimeRef->stop();
  pCmtTimeRef->calculateClockAndMatchRegisterForMicroDelay(1000);  // 1ms
    pCmtTimeRef->setMatchInterruptFuncPtr(IRQTimer);
    pCmtTimeRef->start();
}

void TimeMeasurement::restart() {
    if (time_1ms == 0) {
        start();
    }
}

void TimeMeasurement::stop() {
    pCmtTimeRef->stop();
    time_1ms = 0;
}

// Start Measuring.
// Parameter:
// job_next: -1 The next Job is giving back as Return
TimeMeasurement::measureJob& TimeMeasurement::registerTimedJob(int8_t job_next) {
    measureJob *jobPtr;
    if ((job_next < 0)) {
        jobPtr = &GetNextJob();
    } else {
        jobPtr = &jobs[job_next];
    }
    jobPtr->registered = true;
    jobPtr->time = time_1ms;
    return *jobPtr;
}

void TimeMeasurement::unregisterTimedJob(int8_t jobId) {
    if (jobId <= job_max) {
        jobs[jobId].registered = false;
        jobs[jobId].time = time_1ms;
    }
}

void TimeMeasurement::unregisterTimedJob(measureJob &job) {
    job.registered = false;
    job.time = time_1ms;
}

// Get time difference [ms] since the registration of the job
uint32_t TimeMeasurement::getJobDeltat(uint8_t job_nr) {
    if (jobs[job_nr].registered) {
        return time_1ms - jobs[job_nr].time;
    }
    return 0;
}

uint32_t TimeMeasurement::getJobDeltat(measureJob &job) {
    if (job.registered) {
        return time_1ms - job.time;
    }
    return 0;
}

int TimeMeasurement::getJobIndex(measureJob &job) {
    int jobIndex = 0;
    while (&jobs[jobIndex] != &job) {
        if (++jobIndex > job_max) {
            jobIndex = -1;
            break;
        }
    }
    return jobIndex;
}

void TimeMeasurement::resetJobTime(measureJob &job) {
    job.time = time_1ms;
}

TimeMeasurement::measureJob& TimeMeasurement::GetNextJob(void) {
    uint8_t job_idx = 0;
    while (jobs[job_idx].registered) {
        job_idx++;
    }
    return jobs[job_idx];
}

void TimeMeasurement::IRQTimer() {
    time_1ms++;
}

// Get formated output of the actual value of the job timer.
// Input job_nr - Index to the job that performs the time measurement
//       obuf - pointer to the output string
//       oLen - length of obuf (usually 12 Bytes + Null-Terminator)
//       nFormat - desired output format. Use an element of enumeration format2str
// Output obuf - pointer to the output string with formated text
char* TimeMeasurement::toString(uint8_t job_nr, char *obuf, uint32_t oLen, format2str nFormat) {
    unsigned int h, m, s;
    unsigned int time = getJobDeltat(job_nr);
    unsigned int ms = time % 1000;
    if (nFormat != Raw_Millisecs) {
        time /= 1000;
    }
    switch (nFormat) {
        case Seconds:
        case Secs_with_ms:
        case Raw_Millisecs:
            snprintf(obuf, oLen, "%d", time);
            break;
        default:
            s = time % 60;
            time /= 60;
            m = time % 60;
            h = time / 60;
            snprintf(obuf, oLen, "%02d:%02d:%02d", h, m, s);
            break;
    }

    if ((nFormat == Secs_with_ms) || (nFormat == TimeHHMMSSms)) {
        uint32_t nLen = strlen(obuf);
        snprintf(&obuf[nLen], oLen-nLen,  ".%03d", ms);
    }
    return obuf;
}

}  // namespace AME_SRC
