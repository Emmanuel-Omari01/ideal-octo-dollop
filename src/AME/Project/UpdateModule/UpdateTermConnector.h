/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef SRC_AME_PROJECT_UPDATEMODULE_UPDATETERMCONNECTOR_H_
#define SRC_AME_PROJECT_UPDATEMODULE_UPDATETERMCONNECTOR_H_
#include "UpdateDependencies.h"
#include "SoftwareDeploymentFlow.h"
namespace AME_SRC {



class UpdateTermConnector {
 public:
    explicit UpdateTermConnector(SoftwareDeploymentFlow* deployFlow);
    virtual ~UpdateTermConnector();
    static termCommand* fillConnectionsInTab(termCommand* commandTabPtr);
 private:
    static char* callStartUpdate(char *text);
    static char* callGetUpdateStatus(char *text);
    static termCommand _commandTab[];

 private:
    static SoftwareDeploymentFlow* _deployFlow;
};


} /* namespace AME_SRC */

#endif  // SRC_AME_PROJECT_UPDATEMODULE_UPDATETERMCONNECTOR_H_
