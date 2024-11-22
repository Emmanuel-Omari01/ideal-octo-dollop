/*
 * UpdateModulApi.h
 *
 *  Created on: Jun 27, 2024
 *      Author: plcdev
 */

#ifndef SRC_AME_PROJECT_UPDATEMODULE_UPDATEDEPENDENCIES_H_
#define SRC_AME_PROJECT_UPDATEMODULE_UPDATEDEPENDENCIES_H_

#define AME_TERM_COMMAND_INCLUDE_PATH "../Terminal/TermCommandStruct.h"
#ifdef AME_TERM_COMMAND_INCLUDE_PATH
#include AME_TERM_COMMAND_INCLUDE_PATH
#else
#include <stdint.h>

typedef struct {
    const char *name;               // name of the command
    const char *description;
    char* (*call_adr)(char *code);  // routine to be carried out
    int16_t name_nr;  // if case of name==NULL Number of the text
    int16_t desc_nr;  // in case of description == NULL Number of the text according to the description
}termCommand;

#endif

#define STATE_PROVIDER_INCLUDE_PATHS "../SystemLogic/IEventStateProvider.h"
#ifdef STATE_PROVIDER_INCLUDE_PATHS
#include STATE_PROVIDER_INCLUDE_PATHS
#else
class IEventStateProvider {
 public:
    enum states {
        kIdle,
        kDefault,
        kUnimportantEvent,
        kImportantEvent,
        kUrgentEvent,
        kCriticalEvent,
        kFailur
    };
    virtual IEventStateProvider::states getState() = 0;
};
#endif


namespace AME_SRC {
#ifdef GLOBAL_INCLUDE_PATHs
#include GLOBAL_INCLUDE_PATH
#else
namespace Global {
static char OutBuff[128] = { 0 };
}
#endif



}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_UPDATEMODULE_UPDATEDEPENDENCIES_H_
