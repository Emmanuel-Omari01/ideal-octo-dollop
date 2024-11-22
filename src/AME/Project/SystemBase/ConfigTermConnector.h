/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef AME_PROJECT_SYSTEMBASE_CONFIGTERMCONNECTOR_H_
#define AME_PROJECT_SYSTEMBASE_CONFIGTERMCONNECTOR_H_
#include "../Terminal/TermCommandStruct.h"
#include "../Terminal/TermContextFilter.h"
#include "../CompositeComponents/FoundationModule/ITimeOutWatcher.h"
#include "../PlcCommunication/AliveHandling/MessageForwarding/BroadcastRoutingManager.h"
#include "../Test/UnitSuite/UnitSuiteTests.h"
#include "../SystemLogic/AutostartFlow.h"
namespace AME_SRC {

class ConfigTermConnector {
 public:
    ConfigTermConnector(ITimeOutWatcher *pW, TermContextFilter *contextFilter,
            BroadcastRoutingManager *routingManagerPtr,
            UnitSuiteTests *unitTests,
            AutostartFlow* autostartFlowPtr,
            SerialDriver* modemDrive);
    virtual ~ConfigTermConnector();

    static termCommand* fillConnectionsInTab(termCommand*& commandTabPtr);
    static char* termSendPLCRawData(char *text);

 private:
    static ITimeOutWatcher * _pW;
    static TermContextFilter* _contextFilter;
    static BroadcastRoutingManager *_routingManagerPtr;
    static UnitSuiteTests* _unitTests;
    static AutostartFlow* _autostartFlowPtr;
    static SerialDriver* _modemDrive;

    static char* callGetAnalogPinLevelByIndex(char *commandAndArguments);
    static char* setMemberCheckInterval(char *textInterval);
    static char _outBuffer[];
    static char* setID(char *id);
    static char* getIDTerminal(char *dummy);
    static char* TerminalSetAutoStartFlowMode(char *value);
    static char* TerminalSetTargetNetId(char *value);
    static char* showAliveMapping(char *value);
    static char* termCallEditTimeout(char *value);
    static char* TerminalSetOwnNetworkAddress(char *value);
    static char* TerminalSetTestSuite(char *value);

    static char* callSoftwareReset(char *dummy);
    static char* callGlobalReset(char *dummy);
    static void doSendGlobalReset();

};

} /* namespace AME_SRC */

#endif /* AME_PROJECT_SYSTEMBASE_CONFIGTERMCONNECTOR_H_ */
