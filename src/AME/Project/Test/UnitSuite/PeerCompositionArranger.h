/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef SRC_AME_PROJECT_TEST_UNITSUITE_PEERCOMPOSITIONARRANGER_H_
#define SRC_AME_PROJECT_TEST_UNITSUITE_PEERCOMPOSITIONARRANGER_H_

#include "../../PlcCommunication/ModemInterpreter.h"

namespace AME_SRC {

class PeerCompositionArranger {
 public:
    PeerCompositionArranger();
    virtual ~PeerCompositionArranger();

    void create3PeerY();
    void create3PeerL();
    void create3PeerW();
    void create3PeerLine();
    void create4PeerNetworkSeparation();
    void create5PeerF();
    void create5PeerLineIntersection();
    void create8PeerCornerIntersection();
    void create8PeerDoubleIntersection();
    void create11PeerDoubleGroup();
    void create22PeerScenario();
    void createScenario16();
 private:
    uint8_t* tsrd_prepare_stream(uint8_t *stream, uint32_t streamLen,
            uint8_t ip, uint8_t count);
    uint8_t* tsrd_add_stream(uint8_t *stream, uint16_t dest_ip,
            uint8_t linkQuality);
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TEST_UNITSUITE_PEERCOMPOSITIONARRANGER_H_
