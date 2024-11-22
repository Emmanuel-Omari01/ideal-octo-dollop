/*
 * SCI_Information.h
 *
 *  Created on: 05.04.2022
 *      Author: Tobias Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_TERMINAL_SCI_SCIPARAM_H_
#define SRC_AME_PROJECT_TERMINAL_SCI_SCIPARAM_H_

namespace AME_SRC {

typedef enum { sci1, sci5, sci12 } SCINumber;
typedef enum {
    baud9600,
    baud115200,
    baud230400,
    baud300000,
    baud375000,
    baud500000,
    baud750000
} Baudrate;

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_SCI_SCIPARAM_H_
