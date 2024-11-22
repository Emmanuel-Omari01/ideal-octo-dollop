/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef SRC_AME_PROJECT_TERMINAL_TERMCOMMANDSTRUCT_H_
#define SRC_AME_PROJECT_TERMINAL_TERMCOMMANDSTRUCT_H_

#include <stdint.h>
#include <stddef.h>
namespace AME_SRC {

// Structure for an array with terminal commands
typedef struct  {
    const char *name;               // name of the command
    const char *description;
    char* (*call_adr)(char *code);  // routine to be carried out
    int16_t name_nr;  // if case of name==NULL Number of the text
    int16_t desc_nr;  // in case of description == NULL Number of the text according to the description
}termCommand;

static termCommand NULLCmd = { reinterpret_cast<char *>(0xFFFFFFFF), NULL, NULL, 0, 0 };

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TERMINAL_TERMCOMMANDSTRUCT_H_
