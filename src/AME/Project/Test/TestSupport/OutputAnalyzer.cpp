/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include "OutputAnalyzer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../SystemBase/Global2.h"

namespace AME_SRC {

OutputAnalyzer::OutputAnalyzer() {}

bool OutputAnalyzer::isGlobalOutputEqualTo(const char *expectedOutput) {
    bool isEqual = false;
    size_t udpHeaderOffset = 16;
    size_t length = strlen(expectedOutput);
    char *generatedOutput = new char[length + 1];
    hexToString(Global2::OutBuff + udpHeaderOffset, generatedOutput);
    isEqual = (strncmp(generatedOutput, expectedOutput, length) == 0);
    delete[] generatedOutput;
    return isEqual;
}

// TODO(AME-Team) this function should belong to a converter class. It should be injected into the UnitSuiteTests.
// Function to convert a hex value to a string
void OutputAnalyzer::hexToString(const char *hex, char *result) {
    int len = strlen(hex);
    for (int i = 0; i < len; i += 2) {
        // Extract two hex digits
        char hexPair[3] = { hex[i], hex[i + 1], '\0' };

        // Convert hex digits to an char value
        char charValue = static_cast<char>(strtol(hexPair, NULL, 16));

        // Add the integer value to the result
        const int size = (sizeof("c"));
        snprintf(result+ (i / 2), size, "%c", charValue);
    }
}


OutputAnalyzer::~OutputAnalyzer() {}

} /* namespace AME_SRC */
