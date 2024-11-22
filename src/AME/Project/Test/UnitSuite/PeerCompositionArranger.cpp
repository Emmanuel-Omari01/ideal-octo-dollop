/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include "PeerCompositionArranger.h"

namespace AME_SRC {

PeerCompositionArranger::PeerCompositionArranger() = default;

PeerCompositionArranger::~PeerCompositionArranger() = default;

void PeerCompositionArranger::create3PeerY() {
    uint8_t *stream;
    IDiscoveryManager *pDm;
    pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    // Example 3 with 3 peers
    // coordinator
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 3);
    stream = tsrd_add_stream(stream, 3, 61);
    stream = tsrd_add_stream(stream, 1, 58);
    tsrd_add_stream(stream, 2, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 2);
    stream = tsrd_add_stream(stream, 0, 51);
    tsrd_add_stream(stream, 3, 51);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 1);
    tsrd_add_stream(stream, 0, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 2);
    stream = tsrd_add_stream(stream, 0, 60);
    tsrd_add_stream(stream, 2, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create3PeerL() {
    uint8_t *stream;
    IDiscoveryManager *pDm;
    pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    // example 5 with 3 peers
    // coordinator
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 2);
    stream = tsrd_add_stream(stream, 2, 60);
    stream = tsrd_add_stream(stream, 3, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 1);
    stream = tsrd_add_stream(stream, 0, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 2);
    stream = tsrd_add_stream(stream, 0, 58);
    stream = tsrd_add_stream(stream, 1, 47);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 1);
    stream = tsrd_add_stream(stream, 0, 66);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create3PeerW() {
    uint8_t *stream;
    IDiscoveryManager *pDm;
    pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 3);
    stream = tsrd_add_stream(stream, 1, 63);
    stream = tsrd_add_stream(stream, 2, 52);
    tsrd_add_stream(stream, 3, 61);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 1);
    tsrd_add_stream(stream, 0, 63);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 1);
    tsrd_add_stream(stream, 0, 52);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 1);
    tsrd_add_stream(stream, 0, 61);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create3PeerLine() {
    uint8_t *stream;
    IDiscoveryManager *pDm;
    pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    // Example 4 with 3 peers
    // Coordinator
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 3);
    stream = tsrd_add_stream(stream, 1, 62);
    stream = tsrd_add_stream(stream, 3, 63);
    tsrd_add_stream(stream, 2, 62);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 1);
    stream = tsrd_add_stream(stream, 0, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 3);
    stream = tsrd_add_stream(stream, 0, 66);
    stream = tsrd_add_stream(stream, 1, 60);
    tsrd_add_stream(stream, 3, 60);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 2);
    stream = tsrd_add_stream(stream, 0, 58);
    tsrd_add_stream(stream, 2, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create4PeerNetworkSeparation() {
    uint8_t *stream;
    IDiscoveryManager *pDm;
    pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    // coordinator
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 1);
    stream = tsrd_add_stream(stream, 2, 52);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 1);
    stream = tsrd_add_stream(stream, 3, 57);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 2);
    stream = tsrd_add_stream(stream, 0, 52);
    stream = tsrd_add_stream(stream, 4, 51);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 1);
    stream = tsrd_add_stream(stream, 1, 57);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 4
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 4, 1);
    stream = tsrd_add_stream(stream, 2, 51);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create5PeerF() {
    uint8_t *stream;
    IDiscoveryManager *pDm;
    pDm = ModemInterpreter::getDiscManagerPtr();
    // example 1 with 5 peers
    // coordinator
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 5);
    stream = tsrd_add_stream(stream, 3, 65);
    stream = tsrd_add_stream(stream, 1, 60);
    stream = tsrd_add_stream(stream, 2, 60);
    stream = tsrd_add_stream(stream, 4, 60);
    tsrd_add_stream(stream, 5, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 2);
    stream = tsrd_add_stream(stream, 0, 58);
    tsrd_add_stream(stream, 3, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 4);
    stream = tsrd_add_stream(stream, 3, 58);
    stream = tsrd_add_stream(stream, 0, 56);
    stream = tsrd_add_stream(stream, 5, 56);
    tsrd_add_stream(stream, 1, 46);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 4);
    stream = tsrd_add_stream(stream, 0, 68);
    stream = tsrd_add_stream(stream, 2, 68);
    stream = tsrd_add_stream(stream, 1, 63);
    tsrd_add_stream(stream, 4, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 4
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 4, 3);
    stream = tsrd_add_stream(stream, 2, 52);
    stream = tsrd_add_stream(stream, 3, 51);
    tsrd_add_stream(stream, 0, 47);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 5
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 5, 2);
    stream = tsrd_add_stream(stream, 0, 63);
    tsrd_add_stream(stream, 1, 62);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create5PeerLineIntersection() {
    uint8_t *stream;
    IDiscoveryManager *pDm;
    pDm = ModemInterpreter::getDiscManagerPtr();

    // example 2 with 5 peers
    // coordinator
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 5);
    stream = tsrd_add_stream(stream, 5, 75);
    stream = tsrd_add_stream(stream, 2, 73);
    stream = tsrd_add_stream(stream, 4, 70);
    stream = tsrd_add_stream(stream, 3, 63);
    tsrd_add_stream(stream, 1, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 3);
    stream = tsrd_add_stream(stream, 4, 70);
    stream = tsrd_add_stream(stream, 0, 60);
    tsrd_add_stream(stream, 3, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 5);
    stream = tsrd_add_stream(stream, 5, 80);
    stream = tsrd_add_stream(stream, 4, 68);
    stream = tsrd_add_stream(stream, 0, 66);
    stream = tsrd_add_stream(stream, 3, 65);
    tsrd_add_stream(stream, 1, 51);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 3);
    stream = tsrd_add_stream(stream, 2, 74);
    stream = tsrd_add_stream(stream, 0, 63);
    tsrd_add_stream(stream, 4, 62);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 4
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 4, 4);
    stream = tsrd_add_stream(stream, 1, 87);
    stream = tsrd_add_stream(stream, 2, 84);
    stream = tsrd_add_stream(stream, 0, 72);
    tsrd_add_stream(stream, 5, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 5
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 5, 4);
    stream = tsrd_add_stream(stream, 2, 74);
    stream = tsrd_add_stream(stream, 1, 68);
    stream = tsrd_add_stream(stream, 4, 66);
    tsrd_add_stream(stream, 0, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create8PeerCornerIntersection() {
    uint8_t *stream;
    IDiscoveryManager *pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    // example 4 with 3 peers
    // coordinator
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 1);
    stream = tsrd_add_stream(stream, 1, 5);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 1
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 2);
    stream = tsrd_add_stream(stream, 0, 5);
    stream = tsrd_add_stream(stream, 2, 6);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 2
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 2);
    stream = tsrd_add_stream(stream, 1, 6);
    stream = tsrd_add_stream(stream, 3, 7);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 3
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 5);
    stream = tsrd_add_stream(stream, 2, 7);
    tsrd_add_stream(stream, 4, 8);
    tsrd_add_stream(stream, 5, 9);
    tsrd_add_stream(stream, 6, 10);
    tsrd_add_stream(stream, 7, 11);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 4
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 4, 1);
    stream = tsrd_add_stream(stream, 3, 8);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 5
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 5, 1);
    stream = tsrd_add_stream(stream, 3, 9);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 6
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 6, 1);
    stream = tsrd_add_stream(stream, 3, 10);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 7
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 7, 2);
    stream = tsrd_add_stream(stream, 3, 11);
    stream = tsrd_add_stream(stream, 8, 12);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    // Peer 8
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 8, 1);
    stream = tsrd_add_stream(stream, 7, 12);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create8PeerDoubleIntersection() {
    // look at TestSuitGraph, testStep 1, complex
    // A=0=coordinator ;B=1 ;C=2 ;D=3 ;E=4 ;F=5 ;G=6; H=7 ; I=8 ;J=9
    enum {
        A, B, C, D, E, F, G, H, I, J
    };
    uint8_t *stream;
    IDiscoveryManager *pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, A, 3);
    stream = tsrd_add_stream(stream, B, 3);
    stream = tsrd_add_stream(stream, F, 9);
    tsrd_add_stream(stream, D, 6);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, B, 5);
    stream = tsrd_add_stream(stream, A, 3);
    stream = tsrd_add_stream(stream, F, 9);
    stream = tsrd_add_stream(stream, C, 2);
    stream = tsrd_add_stream(stream, D, 4);
    tsrd_add_stream(stream, E, 9);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, C, 4);
    stream = tsrd_add_stream(stream, B, 2);
    stream = tsrd_add_stream(stream, D, 2);
    stream = tsrd_add_stream(stream, E, 8);
    tsrd_add_stream(stream, G, 9);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, D, 4);
    stream = tsrd_add_stream(stream, A, 6);
    stream = tsrd_add_stream(stream, B, 4);
    stream = tsrd_add_stream(stream, C, 2);
    tsrd_add_stream(stream, G, 9);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, E, 6);
    stream = tsrd_add_stream(stream, B, 9);
    stream = tsrd_add_stream(stream, C, 8);
    stream = tsrd_add_stream(stream, F, 8);
    stream = tsrd_add_stream(stream, G, 7);
    stream = tsrd_add_stream(stream, I, 9);
    tsrd_add_stream(stream, J, 10);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, F, 4);
    stream = tsrd_add_stream(stream, A, 9);
    stream = tsrd_add_stream(stream, B, 9);
    stream = tsrd_add_stream(stream, E, 8);
    tsrd_add_stream(stream, J, 18);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, G, 5);
    stream = tsrd_add_stream(stream, D, 9);
    stream = tsrd_add_stream(stream, C, 9);
    stream = tsrd_add_stream(stream, E, 7);
    stream = tsrd_add_stream(stream, I, 5);
    tsrd_add_stream(stream, H, 4);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, H, 3);
    stream = tsrd_add_stream(stream, G, 4);
    stream = tsrd_add_stream(stream, I, 1);
    tsrd_add_stream(stream, J, 4);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, I, 4);
    stream = tsrd_add_stream(stream, G, 5);
    stream = tsrd_add_stream(stream, H, 1);
    stream = tsrd_add_stream(stream, J, 3);
    tsrd_add_stream(stream, E, 9);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, J, 4);
    stream = tsrd_add_stream(stream, H, 4);
    stream = tsrd_add_stream(stream, I, 3);
    stream = tsrd_add_stream(stream, E, 10);
    tsrd_add_stream(stream, F, 18);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

uint8_t* PeerCompositionArranger::tsrd_prepare_stream(uint8_t *stream, uint32_t streamLen,
        uint8_t ip, uint8_t count) {
    snprintf(reinterpret_cast<char*>(stream), streamLen, "::SD%04d ", ip);
    stream += 9;

    *stream++ = count;
    return stream;
}

uint8_t* PeerCompositionArranger::tsrd_add_stream(uint8_t *stream,
        uint16_t dest_ip, uint8_t linkQuality) {
    uint16_t *pip = reinterpret_cast<uint16_t*>(stream);
    *pip = dest_ip;
    stream += 2;
    *stream++ = linkQuality;
    return stream;
}

void PeerCompositionArranger::create11PeerDoubleGroup() {
    // look at TestSuitGraph, testStep 1, complex
    // A=0=coordinator ;B=1 ;C=2 ;D=3 ;E=4 ;F=5 ;G=6; H=7 ; I=8 ;J=9; K=10; L=11
    enum {
        A, B, C, D, E, F, G, H, I, J, K, L
    };
    uint8_t *stream;
    IDiscoveryManager *pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries
    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, A, 4);
    stream = tsrd_add_stream(stream, E, 60);
    stream = tsrd_add_stream(stream, F, 104);
    stream = tsrd_add_stream(stream, C, 61);
    stream = tsrd_add_stream(stream, B, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, B, 4);
    stream = tsrd_add_stream(stream, F, 73);
    stream = tsrd_add_stream(stream, G, 66);
    stream = tsrd_add_stream(stream, A, 58);
    stream = tsrd_add_stream(stream, C, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, C, 4);
    stream = tsrd_add_stream(stream, F, 97);
    stream = tsrd_add_stream(stream, G, 77);
    stream = tsrd_add_stream(stream, A, 63);
    tsrd_add_stream(stream, B, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, D, 3);
    stream = tsrd_add_stream(stream, G, 47);
    stream = tsrd_add_stream(stream, F, 47);
    stream = tsrd_add_stream(stream, H, 44);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, E, 1);
    stream = tsrd_add_stream(stream, D, 47);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, F, 5);
    stream = tsrd_add_stream(stream, D, 47);
    stream = tsrd_add_stream(stream, G, 56);
    stream = tsrd_add_stream(stream, H, 44);
    stream = tsrd_add_stream(stream, A, 44);
    stream = tsrd_add_stream(stream, C, 44);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, G, 5);
    stream = tsrd_add_stream(stream, D, 47);
    stream = tsrd_add_stream(stream, E, 44);
    stream = tsrd_add_stream(stream, H, 52);
    stream = tsrd_add_stream(stream, F, 51);
    tsrd_add_stream(stream, C, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, H, 5);
    stream = tsrd_add_stream(stream, J, 51);
    stream = tsrd_add_stream(stream, L, 47);
    stream = tsrd_add_stream(stream, K, 47);
    stream = tsrd_add_stream(stream, G, 51);
    stream = tsrd_add_stream(stream, F, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, I, 4);
    stream = tsrd_add_stream(stream, L, 70);
    stream = tsrd_add_stream(stream, J, 60);
    stream = tsrd_add_stream(stream, K, 56);
    tsrd_add_stream(stream, H, 52);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, J, 4);
    stream = tsrd_add_stream(stream, L, 60);
    stream = tsrd_add_stream(stream, I, 60);
    stream = tsrd_add_stream(stream, K, 58);
    tsrd_add_stream(stream, H, 51);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, K, 3);
    stream = tsrd_add_stream(stream, I, 56);
    stream = tsrd_add_stream(stream, H, 47);
    stream = tsrd_add_stream(stream, A, 0);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, L, 2);
    stream = tsrd_add_stream(stream, I, 66);
    stream = tsrd_add_stream(stream, J, 60);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::create22PeerScenario() {
    uint8_t *stream;
    IDiscoveryManager *pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 4);
    stream = tsrd_add_stream(stream, 19, 44);
    stream = tsrd_add_stream(stream, 11, 40);
    stream = tsrd_add_stream(stream, 16, 56);
    stream = tsrd_add_stream(stream, 12, 44);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 1, 1);
    stream = tsrd_add_stream(stream, 0, 99);    // signal quality 0 could be a problem
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 7);
    stream = tsrd_add_stream(stream, 13, 61);
    stream = tsrd_add_stream(stream, 7, 52);
    stream = tsrd_add_stream(stream, 11, 65);
    stream = tsrd_add_stream(stream, 4, 90);
    stream = tsrd_add_stream(stream, 12, 70);
    stream = tsrd_add_stream(stream, 3, 2);
    stream = tsrd_add_stream(stream, 21, 65);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 2);
    stream = tsrd_add_stream(stream, 21, 65);
    stream = tsrd_add_stream(stream, 18, 62);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 4, 5);
    stream = tsrd_add_stream(stream, 7, 65);
    stream = tsrd_add_stream(stream, 8, 56);
    stream = tsrd_add_stream(stream, 13, 44);
    stream = tsrd_add_stream(stream, 2, 47);
    stream = tsrd_add_stream(stream, 15, 47);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 7, 2);
    stream = tsrd_add_stream(stream, 4, 63);
    stream = tsrd_add_stream(stream, 2, 44);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 8, 5);
    stream = tsrd_add_stream(stream, 1, 72);
    stream = tsrd_add_stream(stream, 7, 62);
    stream = tsrd_add_stream(stream, 13, 65);
    stream = tsrd_add_stream(stream, 15, 52);
    tsrd_add_stream(stream, 10, 40);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 9, 5);
    stream = tsrd_add_stream(stream, 22, 68);
    stream = tsrd_add_stream(stream, 21, 60);
    stream = tsrd_add_stream(stream, 18, 52);
    stream = tsrd_add_stream(stream, 15, 60);
    stream = tsrd_add_stream(stream, 3, 47);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 10, 5);
    stream = tsrd_add_stream(stream, 11, 84);
    stream = tsrd_add_stream(stream, 12, 65);
    stream = tsrd_add_stream(stream, 15, 58);
    stream = tsrd_add_stream(stream, 2, 60);
    tsrd_add_stream(stream, 8, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 11, 5);
    stream = tsrd_add_stream(stream, 22, 42);
    stream = tsrd_add_stream(stream, 12, 92);
    stream = tsrd_add_stream(stream, 15, 66);
    stream = tsrd_add_stream(stream, 2, 60);
    tsrd_add_stream(stream, 0, 62);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 12, 6);
    stream = tsrd_add_stream(stream, 19, 61);
    stream = tsrd_add_stream(stream, 0, 42);
    stream = tsrd_add_stream(stream, 11, 85);
    stream = tsrd_add_stream(stream, 10, 63);
    stream = tsrd_add_stream(stream, 15, 52);
    stream = tsrd_add_stream(stream, 13, 2);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 13, 2);
    stream = tsrd_add_stream(stream, 4, 96);
    stream = tsrd_add_stream(stream, 2, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 15, 5);
    stream = tsrd_add_stream(stream, 18, 56);
    stream = tsrd_add_stream(stream, 0, 62);
    stream = tsrd_add_stream(stream, 3, 58);
    stream = tsrd_add_stream(stream, 8, 52);
    tsrd_add_stream(stream, 13, 51);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 16, 1);
    stream = tsrd_add_stream(stream, 0, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 18, 5);
    stream = tsrd_add_stream(stream, 21, 73);
    stream = tsrd_add_stream(stream, 22, 56);
    stream = tsrd_add_stream(stream, 15, 58);
    stream = tsrd_add_stream(stream, 3, 74);
    tsrd_add_stream(stream, 9, 60);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 19, 2);
    stream = tsrd_add_stream(stream, 12, 56);
    stream = tsrd_add_stream(stream, 0, 46);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 21, 5);
    stream = tsrd_add_stream(stream, 18, 62);
    stream = tsrd_add_stream(stream, 22, 51);
    stream = tsrd_add_stream(stream, 3, 62);
    stream = tsrd_add_stream(stream, 15, 56);
    tsrd_add_stream(stream, 9, 52);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 22, 5);
    stream = tsrd_add_stream(stream, 18, 47);
    stream = tsrd_add_stream(stream, 11, 42);
    stream = tsrd_add_stream(stream, 9, 68);
    stream = tsrd_add_stream(stream, 15, 60);
    tsrd_add_stream(stream, 3, 52);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

void PeerCompositionArranger::createScenario16() {
    uint8_t *stream;
    IDiscoveryManager *pDm = ModemInterpreter::getDiscManagerPtr();
    pDm->storeData('C', NULL, 0);   // delete old entries

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 0, 5);
    stream = tsrd_add_stream(stream, 9, 42);
    stream = tsrd_add_stream(stream, 6, 56);
    stream = tsrd_add_stream(stream, 1, 42);
    stream = tsrd_add_stream(stream, 21, 42);
    stream = tsrd_add_stream(stream, 19, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 2, 5);
    stream = tsrd_add_stream(stream, 17, 52);
    stream = tsrd_add_stream(stream, 16, 52);
    stream = tsrd_add_stream(stream, 12, 44);
    stream = tsrd_add_stream(stream, 1, 44);
    stream = tsrd_add_stream(stream, 21, 40);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 3, 4);
    stream = tsrd_add_stream(stream, 22, 66);
    stream = tsrd_add_stream(stream, 18, 58);
    stream = tsrd_add_stream(stream, 10, 58);
    stream = tsrd_add_stream(stream, 11, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 4, 2);
    stream = tsrd_add_stream(stream, 5, 58);
    stream = tsrd_add_stream(stream, 6, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 5, 2);
    stream = tsrd_add_stream(stream, 4, 65);
    stream = tsrd_add_stream(stream, 0, 46);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 6, 1);
    stream = tsrd_add_stream(stream, 0, 52);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 7, 1);
    stream = tsrd_add_stream(stream, 17, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 8, 3);
    stream = tsrd_add_stream(stream, 13, 61);
    stream = tsrd_add_stream(stream, 17, 60);
    stream = tsrd_add_stream(stream, 2, 56);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 9, 5);
    stream = tsrd_add_stream(stream, 21, 85);
    stream = tsrd_add_stream(stream, 19, 65);
    stream = tsrd_add_stream(stream, 1, 52);
    stream = tsrd_add_stream(stream, 11, 42);
    stream = tsrd_add_stream(stream, 8, 40);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 10, 5);
    stream = tsrd_add_stream(stream, 18, 60);
    stream = tsrd_add_stream(stream, 22, 56);
    stream = tsrd_add_stream(stream, 11, 66);
    stream = tsrd_add_stream(stream, 3, 60);
    stream = tsrd_add_stream(stream, 21, 40);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 11, 4);
    stream = tsrd_add_stream(stream, 10, 65);
    stream = tsrd_add_stream(stream, 18, 60);
    stream = tsrd_add_stream(stream, 22, 56);
    stream = tsrd_add_stream(stream, 3, 58);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 12, 2);
    stream = tsrd_add_stream(stream, 7, 60);
    stream = tsrd_add_stream(stream, 2, 87);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 13, 4);
    stream = tsrd_add_stream(stream, 17, 47);
    stream = tsrd_add_stream(stream, 8, 61);
    stream = tsrd_add_stream(stream, 2, 51);
    stream = tsrd_add_stream(stream, 1, 40);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 15, 5);
    stream = tsrd_add_stream(stream, 9, 61);
    stream = tsrd_add_stream(stream, 21, 60);
    stream = tsrd_add_stream(stream, 0, 58);
    stream = tsrd_add_stream(stream, 16, 56);
    stream = tsrd_add_stream(stream, 8, 44);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 16, 5);
    stream = tsrd_add_stream(stream, 17, 40);
    stream = tsrd_add_stream(stream, 23, 62);
    stream = tsrd_add_stream(stream, 8, 47);
    stream = tsrd_add_stream(stream, 2, 44);
    stream = tsrd_add_stream(stream, 1, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 17, 5);
    stream = tsrd_add_stream(stream, 13, 51);
    stream = tsrd_add_stream(stream, 8, 61);
    stream = tsrd_add_stream(stream, 15, 58);
    stream = tsrd_add_stream(stream, 2, 56);
    stream = tsrd_add_stream(stream, 21, 40);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 18, 5);
    stream = tsrd_add_stream(stream, 11, 58);
    stream = tsrd_add_stream(stream, 3, 58);
    stream = tsrd_add_stream(stream, 1, 56);
    stream = tsrd_add_stream(stream, 19, 42);
    stream = tsrd_add_stream(stream, 21, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 19, 5);
    stream = tsrd_add_stream(stream, 22, 42);
    stream = tsrd_add_stream(stream, 15, 58);
    stream = tsrd_add_stream(stream, 8, 44);
    stream = tsrd_add_stream(stream, 2, 42);
    stream = tsrd_add_stream(stream, 11, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 20, 0);
            // no data at peer 20
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 21, 5);
    stream = tsrd_add_stream(stream, 17, 40);
    stream = tsrd_add_stream(stream, 19, 74);
    stream = tsrd_add_stream(stream, 1, 68);
    stream = tsrd_add_stream(stream, 15, 63);
    stream = tsrd_add_stream(stream, 0, 60);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 22, 5);
    stream = tsrd_add_stream(stream, 3, 68);
    stream = tsrd_add_stream(stream, 11, 56);
    stream = tsrd_add_stream(stream, 1, 52);
    stream = tsrd_add_stream(stream, 21, 44);
    stream = tsrd_add_stream(stream, 9, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 23, 1);
    stream = tsrd_add_stream(stream, 25, 77);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 25, 3);
    stream = tsrd_add_stream(stream, 27, 42);
    stream = tsrd_add_stream(stream, 23, 75);
    stream = tsrd_add_stream(stream, 26, 65);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 26, 1);
    stream = tsrd_add_stream(stream, 27, 72);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);

    stream = tsrd_prepare_stream(reinterpret_cast<uint8_t*>(Global2::InBuff), Global2::inBuffMaxLength, 27, 2);
    stream = tsrd_add_stream(stream, 26, 72);
    stream = tsrd_add_stream(stream, 25, 42);
    pDm->storeData('D', reinterpret_cast<uint8_t*>(Global2::InBuff), 0);
}

}  // namespace AME_SRC
