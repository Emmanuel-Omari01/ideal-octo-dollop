/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */
#ifndef SRC_AME_PROJECT_SYSTEMLOGIC_SYSTEMSTATECONTROLLER_H_
#define SRC_AME_PROJECT_SYSTEMLOGIC_SYSTEMSTATECONTROLLER_H_
#include "IEventStateProvider.h"
namespace AME_SRC {

class SystemStateController : public  IEventStateProvider {
 public:
    SystemStateController();
    virtual ~SystemStateController();

    IEventStateProvider::states getState() {
        return state;
    }

    void setState(IEventStateProvider::states newState) {
        state = newState;
    }
 private:
    static IEventStateProvider::states state;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_SYSTEMLOGIC_SYSTEMSTATECONTROLLER_H_
