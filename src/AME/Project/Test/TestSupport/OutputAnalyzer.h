/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */
#ifndef AME_PROJECT_TEST_TESTSUPPORT_OUTPUTANALYZER_H_
#define AME_PROJECT_TEST_TESTSUPPORT_OUTPUTANALYZER_H_

namespace AME_SRC {

class OutputAnalyzer {
 public:
    OutputAnalyzer();
    bool isGlobalOutputEqualTo(const char* expectedOutput);
    virtual ~OutputAnalyzer();
 private:
    void hexToString(const char *hex, char *result);
};

} /* namespace AME_SRC */

#endif /* AME_PROJECT_TEST_TESTSUPPORT_OUTPUTANALYZER_H_ */
