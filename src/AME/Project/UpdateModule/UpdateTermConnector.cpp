/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include <stddef.h>
#include "UpdateTermConnector.h"

namespace AME_SRC {
SoftwareDeploymentFlow * UpdateTermConnector::_deployFlow;

UpdateTermConnector::UpdateTermConnector(SoftwareDeploymentFlow *deployFlow) {
            _deployFlow = deployFlow;
}

UpdateTermConnector::~UpdateTermConnector() {
}

termCommand* UpdateTermConnector::fillConnectionsInTab(termCommand* commandTabPtr) {
    commandTabPtr[1].call_adr = UpdateTermConnector::callStartUpdate;
    commandTabPtr[2].call_adr = UpdateTermConnector::callGetUpdateStatus;
    return commandTabPtr;
}

char* UpdateTermConnector::callStartUpdate(char *text) {
    _deployFlow->start();
    static char userReply[] = {"Update started"};
    return userReply;
}

char* UpdateTermConnector::callGetUpdateStatus(char *text) {
    static char userReply[] = {"callGetUpdateStatus"};
    return userReply;
}

} /* namespace AME_SRC */
