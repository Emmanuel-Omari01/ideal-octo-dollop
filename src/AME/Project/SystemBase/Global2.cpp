/*
 * Global2.cpp
 *
 *  Created on: 21.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#include "Global2.h"
#include <string.h>
namespace AME_SRC {

CompareTimer1 Global2::cmt1 = CompareTimer1(500);


Global2::Global2() {
    // TODO(AME): Auto-generated constructor stub
}

Global2::~Global2() {
    // TODO(AME): Auto-generated destructor stub
}

const char* Global2::pVersion;
char Global2::OutBuff[Global2::outBuffMaxLength];
char Global2::InBuff[Global2::inBuffMaxLength];

void Global2::set_version(const char *p) {
    pVersion = p;   // strncpy(version, p, sizeof(version));
}

const char * Global2::get_version(void) {
    return pVersion;
}

const char *Global2::get_version(char *dummy) {
    return get_version();
}


}  // namespace AME_SRC
