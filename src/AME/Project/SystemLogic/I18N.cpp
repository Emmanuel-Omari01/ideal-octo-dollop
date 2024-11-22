/*
 * I18N.cpp
 *
 *  Created on: May 27, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#include "I18N.h"
#include "I18nEnglish.cpp"

namespace AME_SRC {

I18N::I18N() = default;

I18N::~I18N() = default;

const char *I18N::getText(textNumber nNo) {
    const char* mem = i18nEnglish[nNo];  // get pointer to a text
    return mem;
}

}  // namespace AME_SRC
