/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include "CommandTabProvider.h"

#include <stddef.h>
#include "../SystemLogic/I18N.h"

namespace AME_SRC {

termCommand CommandTabProvider::_updateTab[] = {
  {NULL, NULL, NULL,  TX::dTxtSeparator, TX::dTxtSeparator},
  { NULL, NULL, NULL, TX::cTxtUpdateStart, TX::dTxtUpdateStart },
  { NULL, NULL, NULL, TX::cTxtUpdateStatus, TX::dTxtUpdateStatus},
  NULLCmd,  // end of table (typedef endList)
};


termCommand CommandTabProvider::_progConfigTab[] = {
    {NULL, NULL, NULL, TX::txtMotorCmd, TX::dTxtSeparator},                         // [0]
    {NULL, NULL, NULL, TX::cTxtRelay, TX::dTxtRelay},                               // [1]
    {NULL, NULL, NULL, TX::cTxtAnalogL, TX::dTxtAnalogL},                           // [2]
    {NULL, NULL, NULL, TX::cTxtResetS, TX::dTxtResetS},                             // [3]
    {NULL, NULL, NULL, TX::CmdGlobalReset, TX::DescriptionGlobalReset},             // [4]
    {NULL, NULL, NULL, TX::txtFlashCmd, TX::dTxtSeparator},                         // [5]
    {NULL, NULL, NULL, TX::cTxtEraseF, TX::dTxtEraseF},                             // [6]
    {NULL, NULL, NULL, TX::cTxtSetId, TX::txtEmpty},                                // [7]
    {NULL, NULL, NULL, TX::cTxtGetId, TX::txtEmpty},                                // [8]
    {NULL, NULL, NULL, TX::CmdSetDefaultParams, TX::DescriptSetDefaultsParams},     // [9]
    {NULL, NULL, NULL, TX::cTxtAutoStartFlow, TX::dTxtAutoStartFlow},               // [10]
    {NULL, NULL, NULL, TX::cTxtSetTargetNetID, TX::dTxtSetTargetNetID},             // [11]
    {NULL, NULL, NULL, TX::cTxtSetOwnNetworkAdr, TX::dTxtSetOwnNetworkAdr},         // [12]
    {NULL, NULL, NULL, TX::cTxtMemWT,            TX::dTxtMemWT},                    // [13]
    {NULL, NULL, NULL, TX::cTxtMemRT,            TX::dTxtMemRT},                    // [14]
    {NULL, NULL, NULL, TX::cTxtTestSuite, TX::dTxtTestSuite},                       // [15]
    {NULL, NULL, NULL, TX::cTxtTestSuite, TX::dTxtUnitTestLastTest},                // [16]
    {NULL, NULL, NULL, TX::cTxtExitS, TX::txtEmpty},                                // [17]
    {NULL, NULL, NULL, TX::CmdGetAliveMapping, TX::DescriptAliveMapping},           // [18]
    {NULL, NULL, NULL, TX::CmdEditTimeout, TX::DescriptSetDelay},                   // [19]
    {NULL, NULL, NULL, TX::cTxtTestSpiFlash, TX::dTxtTestSpiFlash},                 // [20]
    NULLCmd,  // end of table (typedef endeList)
  };


CommandTabProvider::CommandTabProvider() { }

CommandTabProvider::~CommandTabProvider() { }

/* Returns a specific command-table with no function bindings*/
termCommand* CommandTabProvider::getCmdTab(CommandTabProvider::tabNames name) {
    termCommand *resultPtr = NULL;
    switch (name) {
    case update:
        resultPtr = _updateTab;
        break;
    case config:
        resultPtr = _progConfigTab;
        break;
    default:
        resultPtr = &NULLCmd;
        break;
    }
    return resultPtr;
}

} /* namespace AME_SRC */
