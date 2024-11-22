/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */
#ifndef AME_PROJECT_SYSTEMLOGIC_IEVENTSTATEPROVIDER_H_
#define AME_PROJECT_SYSTEMLOGIC_IEVENTSTATEPROVIDER_H_

namespace AME_SRC {

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
    virtual void setState(IEventStateProvider::states state) = 0;
};

} /* namespace AME_SRC */

#endif /* AME_PROJECT_SYSTEMLOGIC_IEVENTSTATEPROVIDER_H_ */
