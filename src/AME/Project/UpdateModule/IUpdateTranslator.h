/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */
#ifndef SRC_AME_PROJECT_UPDATEMODULE_IUPDATETRANSLATOR_H_
#define SRC_AME_PROJECT_UPDATEMODULE_IUPDATETRANSLATOR_H_

#include "UpdateMessage.h"
#include <cstdint>

namespace AME_SRC {

class IUpdateTranslator {
 public:
    bool isUpdateTypeValid(char value) {
        switch (value) {
            case static_cast<char>(UpdateType::fragment):
            case static_cast<char>(UpdateType::start):
            case static_cast<char>(UpdateType::error):
                return true;
            default:
                return false;
        }
    }

    virtual bool isUpdateMessage() = 0;
    virtual UpdateMessage& translateMessage(const char *message) = 0;
    virtual void storeMessage(const UpdateMessage& uMessage) = 0;
    virtual UpdateMessage& getNextTranslation() = 0;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_UPDATEMODULE_IUPDATETRANSLATOR_H_
