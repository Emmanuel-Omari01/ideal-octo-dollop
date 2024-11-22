/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include "CommandInterpreter.h"

#include <cstring>
#include <cstdio>

#include "../SystemBase/ProgramConfig.h"
#include "Utils.h"
#include "SerialProfiles/SerialStandardProfile.h"
#include "../StorageManagement/MemoryInformation.h"
#include "../SystemBase/SystemInformation.h"
#include "TermCommandStruct.h"

namespace AME_SRC {

// define and setup internal variables
U8 dump_step = 0;               // step-# for the step-by-step output of terminal lines
U8 dump_stop = 0;               // step-# to stop the output of help texts
bool lLineAutoClear = false;    // true: automatic clearing of the interpreter line

List<void *> CommandInterpreter::cmdListTab;    // <AME_SRC::termCommand*>
TermContextFilter *CommandInterpreter::termFilterPtr_ = NULL;

// command command_tab[2] = {
//  { "ver", &Terminal::TermPrintSoftwareVersion },
//
//  { 0, 0} // end of table
//};


typedef char* (*commandFuncPtr)(const char *code);
// Command commandTable[]={Command("test",&testCommand)};

//------------------------------------------------------------------------------

CommandInterpreter::CommandInterpreter() :
        dump_step(0), dump_stop(0), Dump_Adr(NULL), lLineAutoClear(false), helpTextDisplayed(false) {
}
CommandInterpreter::CommandInterpreter(SerialDriver *serialDriver) :
        dump_step(0), dump_stop(0), Dump_Adr(NULL), lLineAutoClear(false), helpTextDisplayed(false) {
    termFilterPtr_ = new TermContextFilter(serialDriver);
    termFilterPtr_->setContext(TermContextFilter::filterVerboseStandard);
    HelpCommand();
}

// destructor: will be not used
CommandInterpreter::~CommandInterpreter() = default;

//------------------------------------------------------------------------------
// step-by-step output of terminal lines
void CommandInterpreter::TerminalDump() {
// static bool sb_start = false;
  char cbuf[4];
// char cText[40];
  char *PtrText;
  U8 myStep;
  var32 adr;
  int value, j;

  if (dump_step == 0) {   // no step selected?
      return;             // end immediately
  }

  if (termFilterPtr_->txdFree() < 200) {  // are there characters in the output buffer?
      return;
  }

  myStep = dump_step++;   // Remember current step and set next step

  switch (myStep) {
      case dump_L1+0x00:    // Memory-Dump
      case dump_L1+0x01:
      case dump_L1+0x02:
      case dump_L1+0x03:
        adr.all = (U32) Dump_Adr;
        if (adr.word.hw) {
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%04x:", adr.word.hw);
            termFilterPtr_->print(Global2::OutBuff);
        }
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%04x:", adr.word.lw);
        termFilterPtr_->print(Global2::OutBuff);

        PtrText = Global2::OutBuff;
        *PtrText++ = ' ';
        for (j = 0; j <= 15; j++) {     // 16 bytes per row
            termFilterPtr_->putchar(' ');
            value = *Dump_Adr++;        // get one byte
            Utils::writeHex2(&cbuf[0], (int16_t) value);

            termFilterPtr_->print(cbuf);
            if (j == 7) {     // insert space after 8 characters
                termFilterPtr_->putchar(' ');
            }

            // Show character to the corresponding byte
            if ((value < ' ') || (value >= 0x80)) {
                value = '.';
            }
            *PtrText++ = static_cast<char>(value);
        }
        *PtrText = 0;

        termFilterPtr_->println(Global2::OutBuff);
        break;

        /*** place holder for parameter output
        // 20...
        case dump_Parameter:
             if (parameter_dump(false)) {
               dump_step = dump_Parameter;
             } else {
               dump_step = 0;
             }
             break;
        ***/

    // show help on terminal
    case dump_Help:
        termFilterPtr_->print("\rAME T.Hirsch, D.Schulz RK-Tec MX-Software");
        // termFilterPtr_->println("Debug-Version");
        // termFilterPtr_->println("Release-Version");
        // termFilterPtr_->printP("Test-Ausgabe %u, %u, %s", 1, 2, "ok");
        helpTextDisplayed = false;
        break;
    case dump_Help + 1:
        termFilterPtr_->println(Global2::get_version());
        // if (sb_start == false) { // show version on start of the application
        termFilterPtr_->println(PCBPortSelector::versionToString(Global2::OutBuff, Global2::outBuffMaxLength));
        termFilterPtr_->println("");
        termFilterPtr_->println(
                SystemInformation::resetSourceToString(Global2::OutBuff, Global2::outBuffMaxLength));
        // sb_start = true;
        // dump_step = 0;
        break;
    case 102:
        for (int i = 1; i <= MemoryInformation::info_entries; i++) {
            termFilterPtr_->print(MemoryInformation::toString(Global2::OutBuff, Global2::outBuffMaxLength, i));
        }
        break;
    case 103:
        termFilterPtr_->println(TX::ShowThisHelpScreen); break;
    case 104:  // show Board-ID and state of coordinator
        termFilterPtr_->print(TX::cTxtPlcBoardNr);
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, TX::getText(TX::cTxtIsCoordinatorMsg),
                 ProgramConfig::getPlcBoardId(),
                 ProgramConfig::isCoordinator());
        termFilterPtr_->println(Global2::OutBuff);
        // termFilterPtr_->println("System Functions:");
        break;
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
        break;
    case 112:
    case 113:
#if EE_PROM_ACCESS == 0
    case 114:
#else
    case 114: termFilterPtr_->println("i2c save        aktuelle Werte im EEProm speichern"); break;
#endif
    case 115:
    case 116:
        break;
    case 117:  // no break
    case 118: {
        AME_SRC::termCommand *pRef = getNextCmdPtr(myStep == 117);
        if (pRef != NULL) {
            const char *pName, *pDesc;
            pName = pRef->name;
            if (pName == NULL) {
                if (pRef->name_nr >= 0) {
                    pName = const_cast<char*>(TX::getText((TX::textNumber) pRef->name_nr));
                } else {
                    pName = "";
                }
            }
            pDesc = pRef->description;
            if (pDesc == NULL) {
                if (pRef->desc_nr >= 0) {
                    pDesc = const_cast<char*>(TX::getText((TX::textNumber) pRef->desc_nr));
                } else {
                    pName = "";
                }
            }
            snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s   %s", pName, pDesc);
            termFilterPtr_->println(Global2::OutBuff);
            dump_step = 118;
        }
        break;
    }

    case 119: termFilterPtr_->putchar(SerialStandardProfile::RETURN);
              helpTextDisplayed = true; break;

//            dump_step = 120; break;

//  case 130: SioLocalTerm::println("\rDebugging:"); break;
//  case 131: SioLocalTerm::println("debug on|off|single Debugging ein, aus, einzelne Ausgabe"); break;

    default: dump_step = 0;  // handle invalid step
  }  // switch

  if (myStep == dump_stop) {
      dump_step = 0;
  }
}  // TerminalDump
//------------------------------------------------------------------------------
// Delete command line to prevent double interpretation when called again
void CommandInterpreter::Clear_line(char *LinePtr) {
//  if (LinePtr == NULL)
//      LinePtr = cmdline;

  if (LinePtr != NULL) {
      *LinePtr = 0;     // memset(LinePtr, 0, SerialComInterface::bufferSize);
  }
}
//------------------------------------------------------------------------------
// show Memory-Block (RAM)
// Parameter:
// - PtrLine
//------------------------------------------------------------------------------
void CommandInterpreter::Dump_Command(char *PtrLine)
{ U32 sfr_adr;
  var32 adr;
  bool lOk;
  char *p = PtrLine+1;

  if (*p == '-') {    // show last block
      Dump_Adr -= 0x40;
  } else {
      Utils::inHexU32(&PtrLine[2], &sfr_adr, &lOk);
      if (lOk) {
          adr.all = sfr_adr;
          Dump_Adr = reinterpret_cast<U8 *>(adr.all);    // save new address
      }
  }
  // prevent re-interpretation
  Clear_line(PtrLine);

  dump_step = dump_L1;  // start step chain
}

void CommandInterpreter::Dump_Command(uint32_t adr) {
    Dump_Adr = reinterpret_cast<U8 *>(adr);
    dump_step = dump_L1;  // start step chain
}
//------------------------------------------------------------------------------
// Read variable with a size from 1 to 4 and display it on terminal
// The call is made via the terminal by specifying the address of the variable
// Address and specification of '2' or '4' if it is 2 or 4 bytes
// Parameter:
// - PtrLine - pointer to input line
//------------------------------------------------------------------------------
void CommandInterpreter::ReadCommand(char *PtrLine)
{ char cbuf[12];
  char lWort = 0;
  bool lOk;
  void *sfr_adr;
  U32 value;

    if (PtrLine[1] == '2') {    // read word?
        lWort = 1;
        PtrLine++;
    } else if (PtrLine[1] == '4') {  // read long word?
        lWort = 2;
        PtrLine++;
    }

  Utils::inHexU32(&PtrLine[2], reinterpret_cast<U32*>(&sfr_adr), &lOk);
  Utils::writeHex8(&cbuf[0], reinterpret_cast<int>(sfr_adr));
  termFilterPtr_->print(cbuf);
  termFilterPtr_->putchar('=');
    switch (lWort) {
    case 1:
        value = *(reinterpret_cast<U16*>(sfr_adr));
        Utils::writeHex4(&cbuf[0], (int16_t) value);
        break;
    case 2:
        value = *(reinterpret_cast<U32*>(sfr_adr));
        Utils::writeHex8(&cbuf[0], (int16_t) value);
        break;
    default:
        value = *(reinterpret_cast<U8*>(sfr_adr));
        Utils::writeHex2(&cbuf[0], (int16_t) value);
    }
    termFilterPtr_->println(cbuf);
}
//------------------------------------------------------------------------------
// Write variable with a size from 1 to 4
// The call is made via the terminal by specifying the address of the variable
// Address and specification of '2' or '4' if it is 2 or 4 bytes
// Parameter:
// - PtrLine - pointer to input line
//------------------------------------------------------------------------------
void CommandInterpreter::WriteCommand(char *PtrLine)
{ char cbuf[16];
  U8  *sfr_adr;
  U16 *sfr_word;
  U32 *sfr_long;
  U32 value;
  bool lOk;
  int cnt = 0;

  if (PtrLine[1] == '4') {    // w4?
      PtrLine = Utils::inHexU32(&PtrLine[3], reinterpret_cast<uint32_t*>(&sfr_long), &lOk);
      if (lOk) cnt++;
      PtrLine = Utils::inHexU32(PtrLine, &value, &lOk);
      if (lOk) cnt++;
      if (cnt == 2) {
            Utils::writeHex8(&cbuf[0], reinterpret_cast<int>(sfr_long));
            termFilterPtr_->print(cbuf);
            termFilterPtr_->putchar(':');
            termFilterPtr_->putchar('=');
            *sfr_long = value;

            Utils::writeHex8(&cbuf[0], value);
            termFilterPtr_->print(cbuf);
        } else {
            termFilterPtr_->putchar('?');
        }
    } else if (PtrLine[1] == '2') {  // w2?
        PtrLine = Utils::inHexU32(&PtrLine[3], reinterpret_cast<U32*>(&sfr_word), &lOk);
        if (lOk)
            cnt++;
        PtrLine = Utils::inHexU32(PtrLine, &value, &lOk);
        if (lOk)
            cnt++;
        if (cnt == 2) {
            Utils::writeHex8(&cbuf[0], reinterpret_cast<int>(sfr_word));
            termFilterPtr_->print(cbuf);
            termFilterPtr_->putchar(':');
            termFilterPtr_->putchar('=');
            *sfr_word = (U16) value;

            Utils::writeHex4(&cbuf[0], (int16_t) value);
            termFilterPtr_->print(cbuf);
        } else {
            termFilterPtr_->putchar('?');
        }
    } else {
        PtrLine = Utils::inHexU32(&PtrLine[2], reinterpret_cast<U32*>(&sfr_adr), &lOk);
        if (lOk)
            cnt++;
        PtrLine = Utils::inHexU32(PtrLine, &value, &lOk);
        if (lOk)
            cnt++;
        if (cnt == 2) {
            Utils::writeHex8(&cbuf[0], reinterpret_cast<int>(sfr_adr));
            termFilterPtr_->print(cbuf);
            termFilterPtr_->putchar(':');
            termFilterPtr_->putchar('=');
            *sfr_adr = (U8) value;

            Utils::writeHex2(&cbuf[0], (int16_t) value);
            termFilterPtr_->print(cbuf);
        } else {
            termFilterPtr_->putchar('?');
        }
    }

  termFilterPtr_->putchar(SerialStandardProfile::RETURN);
}
//------------------------------------------------------------------------------
// Write block in RAM memory.
// The call is made via the terminal with the address of the block address, a padding
// byte and the memory size.
// Attention: there is no monitoring of the memory size, so relevant memory areas may be
// overwritten and thus the software may become unstable.
// Parameter:
// - PtrLine - pointer to input line
//------------------------------------------------------------------------------
void CommandInterpreter::FillCommand(char *PtrLine) {
    U8 *sfr_adr;
    U32 value, hAnz;
    U32 cnt;

    cnt = sscanf(&PtrLine[2], "%x %x %x", reinterpret_cast<int*>(&sfr_adr),
            (unsigned int*) &hAnz, (unsigned int*) &value);
    if (cnt == 3) {
        for (cnt = 1; cnt <= hAnz; cnt++) {
            *sfr_adr++ = (U8) value;
        }
        termFilterPtr_->putchar('o');
        termFilterPtr_->putchar('k');
    } else {
        termFilterPtr_->putchar('?');
    }

    termFilterPtr_->putchar(SerialStandardProfile::RETURN);
}
//------------------------------------------------------------------------------
// Trigger the output of the help text on the terminal
void CommandInterpreter::HelpCommand(void) {
    helpTextDisplayed = false;
    dump_step = dump_Help;  // start step chain
    dump_stop = 0;
}
//------------------------------------------------------------------------------
/* Command line interpreter
   The command line interpreter checks the first characters of the command line
   a valid code. The corresponding routines are then called
------------------------------------------------------------------------------*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Interpret a command table and execute an action
// Parameter:
// cmd_tab : Table in which the command is located
// PtrIn1  : pointer to input line
// lSkip   : Flag that determines whether the executing routine reads the entire input line
//           receives (lSkip = false) or the calling command is ignored (lSkip = true)
//           and the calling routine receives the input line starting from the command// Return:
// true    - the command was found and the appropriate routine was executed
// false   - the command was NOT found and no further action was taken
bool CommandInterpreter::interpret_tab(char *PtrIn1, __attribute__((unused)) bool lSkip) {
    // Browse list of commands
    const char *pName;
    Node<void *> *pList = cmdListTab.getHead();
    while (pList) {
        termCommand *pRef = reinterpret_cast<termCommand*>(pList->getData());
        while (true) {
            endeList *pEndeList = reinterpret_cast<endeList*>(pRef);
            if (pEndeList->ende == 0xFFFFFFFF) {
                break;
            }
            pName = pRef->name;     // name = command
            if (pName == NULL) {
                if (pRef->name_nr >= 0) {
                    pName = reinterpret_cast<const char *>(TX::getText((TX::textNumber) pRef->name_nr));
                }
            }
            if (strstr(PtrIn1, pName) == PtrIn1) {
                if (pRef->call_adr) {
                    termFilterPtr_->println((pRef->call_adr)(PtrIn1));
                    return true;
                } else {
                    termFilterPtr_->println("Name Found but no Function is linked!");
                    return false;
                }
            }
            pRef++;
        }
        pList = pList->getNext();
    }
  return false;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Interpret the command line of the local terminal
void CommandInterpreter::ExecuteLocalCmd(void) {
    char *p;

    p = termFilterPtr_->get_buffer();  // fetch command line
    if (p) {
        DoExecuteCmd(p, lLineAutoClear);
    }
}

AME_SRC::termCommand* CommandInterpreter::getNextCmdPtr(bool lStart) {
    enum state { innen, aussen };
    static Node<void*> *pList = NULL;
    static uint8_t status = aussen;
    static AME_SRC::termCommand *pRef = NULL;
    endeList *pEndeList;

    if (lStart) {
        pList = cmdListTab.getHead();
        status = aussen;
    } else {
        if (status == aussen) {
            pList = pList->getNext();
        } else {
            pRef++;
        }
    }

    for (int i = 1; i <= 2; i++) {
        if (status == aussen) {
            if (pList) {
                pRef = reinterpret_cast<AME_SRC::termCommand*>(pList->getData());
                status = innen;
            } else {
                break;
            }
        }
        if (status == innen) {
            pEndeList = reinterpret_cast<endeList*>(pRef);
            if (pEndeList->ende == 0xFFFFFFFF) {
                status = aussen;
                pList = pList->getNext();
                continue;
            }
            return pRef;
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
// Interpret single terminal line
// Parameter:
// - PtrLine    - pointer to the beginning of an input line
// - lClear     - clear line to prevent re-interpretation
//------------------------------------------------------------------------------
void CommandInterpreter::DoExecuteCmd(char *PtrIn1, bool lClear) {
    char *PtrIn2;

    PtrIn2 = PtrIn1;    // use copy of the pointer
    if (interpret_tab(PtrIn2, false)) {
        return;
    }

    switch (*PtrIn1) {
        case 'H':   // help
        case 'h':
        case '?':
            HelpCommand();
            break;
    case 'd':  // Memory-Dump
            Dump_Command(PtrIn1);
            break;
    case 'f':
            FillCommand(PtrIn1);
            break;
    case 'r':
            ReadCommand(PtrIn1);
            lClear = 0;             // make command repeatable
            break;
    case 'w':
            WriteCommand(PtrIn1);
            lClear = 0;             // make command repeatable
            break;
    case 0: break;                  // line without characters
    default:
        termFilterPtr_->println("\r?");
    }  // switch

    // Clear line if necessary
    if (lClear) {
        Clear_line(PtrIn2);
    }
}

void CommandInterpreter::addCmdList(AME_SRC::termCommand *cmdList) {
    cmdListTab.add(cmdList);
}

}  // namespace AME_SRC

