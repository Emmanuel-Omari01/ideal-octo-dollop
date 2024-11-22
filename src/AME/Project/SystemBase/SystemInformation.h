/*
 * SystemInformation.h
 *
 *  Created on: Feb 12, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_SYSTEMINFORMATION_H_
#define SRC_AME_PROJECT_SYSTEMBASE_SYSTEMINFORMATION_H_

namespace AME_SRC {

class SystemInformation {
 public:
    SystemInformation();
    virtual ~SystemInformation();
    static char* resetSourceToString(char *output, unsigned int maxlen);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMBASE_SYSTEMINFORMATION_H_
