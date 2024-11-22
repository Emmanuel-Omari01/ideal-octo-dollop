/*
 * VersionKey.h
 *
 *  Created on: Feb 8, 2024
 *      Author: Detlef Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_SYSTEMBASE_VERSIONKEY_H_
#define SRC_AME_PROJECT_SYSTEMBASE_VERSIONKEY_H_

// Key for the PLC communication (PSK = Pre shared Key)
// Please enter 16 bytes separated by commas in hexadecimal notation

//              1234567890123456
//              17.04.2024 V0.10
#define AME_PSK 0x31, 0x37, 0x2E, 0x30, 0x34, 0x2E, 0x32, 0x30, 0x32, 0x34, 0x20, 0x56, 0x30, 0x2E, 0x31, 0x30

#endif  // SRC_AME_PROJECT_SYSTEMBASE_VERSIONKEY_H_
