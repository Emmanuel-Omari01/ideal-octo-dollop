/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef SRC_AME_PROJECT_TERMINAL_COMMANDINTERPRETER_H_
#define SRC_AME_PROJECT_TERMINAL_COMMANDINTERPRETER_H_

#include "../SystemBase/Global.h"
#include "../HelpStructures/List.h"
#include "SerialDrivers/SerialDriver.h"
#include "TermContextFilter.h"
#include "TermCommandStruct.h"

namespace AME_SRC {

class CommandInterpreter {
 public:
    // Structure for the end of an command line table: same size as the structure AME_SRC::termCommand
    struct endeList {
        uint32_t ende;
        uint32_t dummy[3];
    };

    static List<void*> cmdListTab;  // <AME_SRC::termCommand*>

 private:
    U8 dump_step;               // step number in TerminalDump
    U8 dump_stop;

    U8 *Dump_Adr;               // last address in Memory-Dump

    bool lLineAutoClear;        // true: automatic clear of the line to interpret
    bool helpTextDisplayed;
    void Dump_Command(char *PtrLine);   // also public version available
    void ReadCommand(char *PtrLine);
    void WriteCommand(char *PtrLine);
    void FillCommand(char *PtrLine);

    static TermContextFilter *termFilterPtr_;

// named number for an step according to dump_step
#define dump_L1         10  // ..14
#define dump_Parameter  20
#define dump_Help       100

 private:
    AME_SRC::termCommand* getNextCmdPtr(bool lStart);

 public:
    CommandInterpreter();
    explicit CommandInterpreter(SerialDriver *serialDriver);
    virtual ~CommandInterpreter();      // Destructor, pro form
    void TerminalDump();
    void HelpCommand();
    void Dump_Command(uint32_t adr);
    void Clear_line(char *LinePtr);
    bool interpret_tab(char *PtrIn1, bool lSkip);
    void ExecuteLocalCmd();
    void DoExecuteCmd(char *PtrIn1, bool lClear);
    void addCmdList(AME_SRC::termCommand *cmdList);

    bool isHelpTextDisplayed() const {
        return helpTextDisplayed;
    }
};  // class

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_COMMANDINTERPRETER_H_
