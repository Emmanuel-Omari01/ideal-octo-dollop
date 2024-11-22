/****************************************************************/
/*                                                              */
/*      PROJECT NAME :  PLCField                                */
/*      FILE         :  PLCF.cpp                                */
/*      DESCRIPTION  :  Main Program                            */
/*      Author: Tobias Hirsch, AME                              */
/*      Copyright (c) 2021 Andreas Müller electronic GmbH (AME) */
/*                                                              */
/****************************************************************/

#ifdef CPPAPP
extern "C" {
#endif
#include "../../../Renesas/Generated/general/r_smc_entry.h"
#ifdef CPPAPP
}
#endif

#include "../SystemBase/MainLoop.h"
#include "../SystemBase/MicrocontrollerConfig.h"
#include "../SystemBase/ProgramConfig.h"
#include "../SystemBase/VersionName.h"


#ifdef CPPAPP
// Initialize global constructors
extern void __main() {
  static int initialized;
  if (!initialized) {
      typedef void (*pfunc) ();
      extern pfunc __ctors[];
      extern pfunc __ctors_end[];
      pfunc *p;

      initialized = 1;
      for (p = __ctors_end; p > __ctors; )
    (*--p) ();
    }
}
#endif

using namespace AME_SRC;

void runBootWith(MainLoop &mainLoopRef);
void runApplicationWith(MainLoop &mainLoopRef);

int main() {
    // Initial Configurations
    ProgramConfig config = ProgramConfig();
    Global2::set_version(reinterpret_cast<const char *>(mxSoftwareVersion));
    MicrocontrollerConfig::initCpuClockGenerator();
    MicrocontrollerConfig::createCMT0();

    MainLoop mainLoop;
    config.getTerminalFilter()->println(mxSoftwareVersion);

    SerialComInterface::getInstance(sci12)->popTXD();
    runBootWith(mainLoop);
    runApplicationWith(mainLoop);
}

/**
 * Processing the boot process of the CPX modem
 * Here the firmware is transferred from the host CPU (RX111) into the modem
 * via a UART (SCI5) interface.
 * @param mainLoopRef Class for managing the time slots
 */
void runBootWith(MainLoop &mainLoopRef) {
    mainLoopRef.setExecution1microPtr(&ProgramConfig::bootWindow100micro);
    mainLoopRef.setExecution50msPtr(&ProgramConfig::bootWindow50ms);
    mainLoopRef.start();
    mainLoopRef.executeUntilStop();
}

// void DEBUGTest() {
//  static SerialStandardProfile serialProfile =SerialStandardProfile();
//  static SerialDriver termDrive =
//            SerialDriver(SerialComInterface::getInstance(sci12,
//                         baud115200, serialProfile));
//  static Testsuite test(&termDrive);
//  test.runTests();
//}

/**
 * Loop to process the customer application
 * @param mainLoopRef Class for managing the time slots
 */
void runApplicationWith(MainLoop &mainLoopRef) {
    mainLoopRef.setExecution1microPtr(0);
    mainLoopRef.setExecution50msPtr(&ProgramConfig::window50ms);
    mainLoopRef.setExecution1msPtr(&ProgramConfig::window1ms);
    mainLoopRef.setExecution5msPtr(&ProgramConfig::window5ms);
    mainLoopRef.setExecution1sPtr(&ProgramConfig::window1s);
    mainLoopRef.setExecution60sPtr(&ProgramConfig::window60s);
    mainLoopRef.start();
    mainLoopRef.executeUntilStop();
}
