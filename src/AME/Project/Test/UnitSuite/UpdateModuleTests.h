/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

    Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
    Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
    Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
    Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
    gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#ifndef SRC_AME_PROJECT_TEST_UNITSUITE_UPDATEMODULETESTS_H_
#define SRC_AME_PROJECT_TEST_UNITSUITE_UPDATEMODULETESTS_H_

#include <cstdint>

#include "../../Terminal/TermContextFilter.h"


namespace AME_SRC {

class UpdateModuleTests {
 public:
    explicit UpdateModuleTests(TermContextFilter* term);
    virtual ~UpdateModuleTests();
    void test018TranslatePlcStartUpdate();
    void test019TranslateUFragment();
    void test020TranslateWrongType();

    void test021TranslatorReStoredMessage();
    void test022TranslatorReStoreFifo();
    void test023TranslatorReStoreFifoWhenOverflow();

    void test024StorageControlLocalFlashSave();
    void test025StorageControlInvalidBaseAddress();

    void test015StartUpdateDeployProcess();
    void test016RepeatUpdateAfterNack();
    void test010BlockUpdateDuringEventMode();
    void test013BlockUpdateIfNotProcessingUpdate();

 private:
    int sumFieldValues(uint8_t *fieldPtr, uint8_t size);
    TermContextFilter  *termFilterPtr_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_TEST_UNITSUITE_UPDATEMODULETESTS_H_
