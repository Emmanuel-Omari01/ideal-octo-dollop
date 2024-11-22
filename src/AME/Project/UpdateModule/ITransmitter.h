/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef SRC_AME_PROJECT_SOFTWAREUPDATE_ITRANSMITTER_H_
#define SRC_AME_PROJECT_SOFTWAREUPDATE_ITRANSMITTER_H_
#include <stdint.h>
namespace AME_SRC {

class ITransmitter {
 public:
    virtual void transmitt(const uint8_t *message, uint8_t length, uint16_t adressIdentifer, char options[5]) = 0;
};

} /* namespace AME_SRC */

#endif /* SRC_AME_PROJECT_SOFTWAREUPDATE_ITRANSMITTER_H_ */
