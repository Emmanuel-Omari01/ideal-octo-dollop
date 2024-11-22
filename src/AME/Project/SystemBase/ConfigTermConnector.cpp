/**
 *    © Copyright 2019 Andreas Müller Electronic(AME) – Urheberrechtshinweis

 Alle Inhalte dieser , insbesondere Text und Quellcode , sind urheberrechtlich geschützt.
 Das Urheberrecht liegt, soweit nicht ausdrücklich anders gekennzeichnet, bei der Firma AME.
 Bitte richten Sie sich an die Firma AME , falls Sie die Inhalte dieses Quellcodes verwenden möchten.
 Wer gegen das Urheberrecht verstößt (z.B. Quellcode oder Texte unerlaubt kopiert), macht sich
 gem. §§ 106 ff UrhG strafbar, wird zudem kostenpflichtig abgemahnt und muss Schadensersatz leisten (§ 97 UrhG).
 */

#include <stdio.h>

#include "ConfigTermConnector.h"
#include "../SignalProcessing/Aktorik/AdditionalOutputDriver.h"
#include "../HelpStructures/CharOperations.h"
#include "../SignalProcessing/AnalogPinHandling/AnalogPin.h"
#include "../CompositeComponents/FoundationModule/ITimeOutWatcher.h"
#include "../SystemLogic/I18N.h"
#include "PCBPortSelector.h"
#include "Global2.h"
#include "MicrocontrollerConfig.h"
#include "../StorageManagement/SimpleFlashEditor.h"
#include "../SystemLogic/AutostartFlow.h"
#include "../Terminal/FlashAccessHelper.h"

namespace AME_SRC {

char ConfigTermConnector::_outBuffer[64];

ITimeOutWatcher *ConfigTermConnector::_pW = NULL;
TermContextFilter *ConfigTermConnector::_contextFilter = NULL;
BroadcastRoutingManager *ConfigTermConnector::_routingManagerPtr = NULL;
UnitSuiteTests *ConfigTermConnector::_unitTests = NULL;
AutostartFlow *ConfigTermConnector::_autostartFlowPtr = NULL;
SerialDriver *ConfigTermConnector::_modemDrive = NULL;

ConfigTermConnector::ConfigTermConnector(ITimeOutWatcher *pW,
        TermContextFilter *contextFilter,
        BroadcastRoutingManager *routingManagerPtr,
        UnitSuiteTests* unitTests,
        AutostartFlow* autostartFlowPtr,
        SerialDriver* modemDrive) {
    _pW = pW;
    _contextFilter = contextFilter;
    _routingManagerPtr = routingManagerPtr;
    _unitTests = unitTests;
    _modemDrive = modemDrive;
}

ConfigTermConnector::~ConfigTermConnector() {
}

termCommand* ConfigTermConnector::fillConnectionsInTab(termCommand*& commandTabPtr) {
    char* (*cmdOrder[])(char *code) = { NULL,                           // [0]
        AdditionalOutputDriver::callRelayToggle,                        // [1]
        ConfigTermConnector::callGetAnalogPinLevelByIndex,              // [2]
        ConfigTermConnector::callSoftwareReset,                         // [3]
        ConfigTermConnector::callGlobalReset,                           // [4]
        NULL,                                                           // [5]
        SimpleFlashEditor::eraseDataFlash,                              // [6]
        ConfigTermConnector::setID,                                     // [7]
        ConfigTermConnector::getIDTerminal,                             // [8]
        SimpleFlashEditor::setDefaultSysParameter,                      // [9]
        ConfigTermConnector::TerminalSetAutoStartFlowMode,              // [10]
        ConfigTermConnector::TerminalSetTargetNetId,                    // [11]
        ConfigTermConnector::TerminalSetOwnNetworkAddress,              // [12]
        SimpleFlashEditor::memoryWriteTest,                             // [13]
        SimpleFlashEditor::memoryReadTest,                              // [14]
        ConfigTermConnector::TerminalSetTestSuite,                      // [15]
        ConfigTermConnector::TerminalSetTestSuite,                      // [16] (2x !)
        AutostartFlow::exitAutoStart,                                   // [17]
        ConfigTermConnector::showAliveMapping,                          // [18]
        ConfigTermConnector::termCallEditTimeout,                       // [19]
        FlashAccessHelper::spiFlashTestInterface,                       // [20]
    };
    int cmdAmount = 21;
    for (int i = 0; i < cmdAmount; i++) {
        if (&commandTabPtr[i] == &NULLCmd) {
           break;
        } else {
            commandTabPtr[i].call_adr = cmdOrder[i];
        }
    }
    return commandTabPtr;
}

/***
 * This method evaluates one of the 1-4 analog signals based on the
 * passed index and returns a corresponding message.
 * @param commandAndArguments Command and index of the analog signal
 * @return Message with analog value
 */
char* ConfigTermConnector::callGetAnalogPinLevelByIndex(
        char *commandAndArguments) {
    uint8_t pinIndex = charOperations::commandArgumentToNumber(
            commandAndArguments);
    AnalogPin *myPin = PCBPortSelector::getAnalogPin(pinIndex);
    if (PCBPortSelector::isValidAnalogPin(myPin)) {
        double pinCurrent = (myPin->read() / static_cast<double>(4096)) * 3.3;

        snprintf(_outBuffer, Global2::outBuffMaxLength,
                TX::getText(TX::cTxtAnalogPinMsg), pinIndex, pinCurrent);
    } else {
        snprintf(_outBuffer, Global2::outBuffMaxLength,
                TX::getText(TX::HardwareReferenceError));
    }
    return _outBuffer;
}

char* ConfigTermConnector::callSoftwareReset(
        __attribute__((unused)) char *dummy) {
    MicrocontrollerConfig::softwareReset();
    return const_cast<char*>("Reset Triggert!");
}

char* ConfigTermConnector::callGlobalReset(
        __attribute__((unused)) char *dummy) {
    uint8_t simpleShortTOutID = 7;
    _pW->clearTimeout(simpleShortTOutID);
    ITimeOutWatcher::timeOutInfo *pToi = _pW->findTimeOutWithID(
            simpleShortTOutID);
    if (pToi == NULL) {
        ITimeOutWatcher::timeOutInfo toi;
        ITimeOutWatcher::createDefaultTimeOut(&toi, simpleShortTOutID,
                &doSendGlobalReset, NULL);
        toi.timeCounter = 2;            //  2 * 0.05s = 100ms
        toi.delay = 40;                 // 40 * 0.05s = 2000ms
        toi.timeOutCounter = 5;
        toi.timeOutAmount = 5;
        _pW->registerTimeout(toi);
        return const_cast<char*>(TX::getText(TX::GlobalResetMessage));
    }
    return const_cast<char*>("Global Reset Triggert!");
}

// Routine for the globalReset-Function:
// sends n times the global reset command into the PLC network and performs a reset on the own device
void ConfigTermConnector::doSendGlobalReset() {
    uint8_t simpleShortTOutID = 7;
    ITimeOutWatcher::timeOutInfo *pToi = _pW->findTimeOutWithID(simpleShortTOutID);
    if (pToi) {
        if (pToi->timeOutCounter <= 1) {
            MicrocontrollerConfig::softwareReset();
            return;
        }
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "%s%cGlobalReset",
                ModemInterpreter::commandPrefix,
                ModemInterpreter::GlobalResetRequest);
        ModemController::triggerSendBroadCastDataWithLengthToGroupeIP(
                (const uint8_t*) Global2::OutBuff,
                (uint8_t) strlen(Global2::OutBuff), IRoutingManager::BCGlobalIP,
                false, TermContextFilter::filterRoutingInfo);
    }
}


char* ConfigTermConnector::setID(char *id) {
    char *textPtr, *savePtr;
    textPtr = strtok_r(id, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    uint16_t adress = (uint16_t) atoi(textPtr);
    ModemController::setMacShortAddress(adress);
    SimpleFlashEditor::setSysParamID(adress);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::cTxtSetIDMsg), adress);
    return Global2::OutBuff;
}

// Show device ID on terminal
char* ConfigTermConnector::getIDTerminal(__attribute__((unused)) char *dummy) {
    uint16_t my_id = SimpleFlashEditor::getBoardID();
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::ShowIDMessage), my_id);
    return Global2::OutBuff;
}

char* ConfigTermConnector::TerminalSetAutoStartFlowMode(char *value) {
    unsigned int mode;
    int32_t cnt;
    const char *txt = charOperations::skip_token(value, ' ');
    cnt = sscanf(txt, "%u", &mode);
    if (cnt >= 1) {
        SimpleFlashEditor::setAutoStartFlowMode((uint16_t) mode);
    } else {
        mode = (uint32_t) SimpleFlashEditor::getAutoStartFlowMode();
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtValueSet), TX::getText(TX::cTxtAutoStartFlow),
            mode);
    return Global2::OutBuff;
}

char* ConfigTermConnector::TerminalSetTargetNetId(char *value) {
    char *textPtr, *savePtr;
    textPtr = strtok_r(value, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    uint16_t mode = (uint16_t) atoi(textPtr);
    SimpleFlashEditor::setTargetNetId(mode);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtValueSet), TX::getText(TX::cTxtSetTargetNetID),
            mode);
    return Global2::OutBuff;
}

char* ConfigTermConnector::showAliveMapping(
        __attribute__((unused)) char *value) {
    _contextFilter->println(_routingManagerPtr->showData(0));
    return const_cast<char*>(TX::getText(TX::EndOfTableMessage));
}

char* ConfigTermConnector::TerminalSetOwnNetworkAddress(char *value) {
    char *textPtr, *savePtr;
    textPtr = strtok_r(value, " ", &savePtr);
    textPtr = strtok_r(NULL, " ", &savePtr);
    uint16_t mode = (uint16_t) atoi(textPtr);
    SimpleFlashEditor::setOwnNetworkAddress(mode);
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxtValueSet),
            TX::getText(TX::cTxtSetOwnNetworkAdr), mode);
    return Global2::OutBuff;
}

char* ConfigTermConnector::TerminalSetTestSuite(char *value) {
    unsigned int val1, val2;
    int32_t cnt;
    const char *txt = charOperations::skip_token(value, ' ');
    char *ret;
    ret = strstr(txt, TX::getText(TX::cTxtUnitTestLastTest));
    if (ret == txt) {
        // specify if the last unit test is running
        bool set;
        txt = charOperations::skip_token(txt, ' ');
        ret = strstr(txt, TX::getText(TX::txtTrue));
        if (ret == txt) {
            set = true;
            _unitTests->setLastUnitTest(set);
        } else {
            ret = strstr(txt, TX::getText(TX::txtFalse));
            if (ret == txt) {
                set = false;
                _unitTests->setLastUnitTest(set);
            } else if (strstr(txt, "?")) {
                set = _unitTests->getLastUnitTest();
            } else {
                return const_cast<char*>(TX::getText(TX::IncorrectEntry));
            }
        }
        snprintf(Global2::OutBuff, Global2::outBuffMaxLength, "\r%s = %s",
                TX::getText(TX::cTxtUnitTestLastTest), (set)? TX::getText(TX::txtTrue):TX::getText(TX::txtFalse));
        return Global2::OutBuff;
    }
    cnt = sscanf(txt, "%u %u", &val1, &val2);
    if (cnt >= 2) {
        _unitTests->setTestParameter(
                (uint8_t) val2);
    } else {
        val2 = _unitTests->getTestParameter();
    }
    if (cnt >= 1) {
        _unitTests->setTestEnabled(
                static_cast<bool>(val1));
        if (val1) {
            _autostartFlowPtr->restartTestSuite();
        }
    } else {
        val1 = _unitTests->getTestEnabled();
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::dTxt2ValuesSet), TX::getText(TX::cTxtTestSuite),
            val1, val2);
    return Global2::OutBuff;
}

char* ConfigTermConnector::termCallEditTimeout(char *value) {
    bool fehler = false;
    unsigned int val1, val2;
    int32_t cnt;
    const char *txt = charOperations::skip_token(value, ' ');
    cnt = sscanf(txt, "%u %u", &val1, &val2);
    if (cnt >= 2) {
        fehler = !_pW->editTimeoutdelayForID((uint8_t) val1, (uint8_t) val2);
    } else if (cnt == 1) {
        ITimeOutWatcher::timeOutInfo *toi = _pW->findTimeOutWithID((uint8_t) val1);
        if (toi) {
            val2 = toi->delay;
        } else {
            fehler = true;
        }
    } else {
        fehler = true;
    }
    if (fehler) {
        return const_cast<char*>(TX::getText(TX::IncorrectEntry));
    }
    snprintf(Global2::OutBuff, Global2::outBuffMaxLength,
            TX::getText(TX::TimeOutSetMessage), val1, val2);
    return Global2::OutBuff;
}

/* Send row data immediately to the PLC-Modem.
 * The data must start and end with 0x7E.
 * This is already checked within this routine.
 * The string should be (08.12.23) shorter than 256 characters, because the command interpreter does not accept longer entries.
 * Example for a string with raw data and correct checksum:
 * sendPLCRaw 7E 00 01 04 00 00 36 60 00 00 00 00 0E 11 0F FE 80 00 00 00 00 00 00 00 02 00 FF FE 00 00 01 FF 02 00 00 00 00 00 00 00 00 00 00 00 00 00 FF AB CD F0 BF 00 0E 41 F1 48 61 6C 6C 6F 00 00 09 01 46 D5 C0 8E 7E
 */
char* ConfigTermConnector::termSendPLCRawData(char *text) {
    const uint8_t delimiter = 0x7E;  // Raw data have to be started and ended with 0x7E
    const char *txt = charOperations::skip_token(text, ' ');

    const char *pCopy = reinterpret_cast<const char*>(txt);
    int inByte, lastByte;
    int cnt = sscanf(pCopy, " %02X", &inByte);
    if (cnt == 1 && inByte == delimiter) {
        inByte = 0;
        while (cnt == 1) {
            pCopy += 3;
            cnt = sscanf(pCopy, " %02X", &lastByte);
            if (cnt == 1 && lastByte < 256) {
                inByte = lastByte;
            }
        }
        if (inByte == delimiter) {
            pCopy = reinterpret_cast<const char*>(txt);
            while (true) {
                cnt = sscanf(pCopy, " %02X", &inByte);
                if (cnt == 1) {
                    _modemDrive->getSciPtr()->pushTXD(static_cast<char>(inByte));
                } else {
                    break;
                }
                pCopy += 3;
            }
            _modemDrive->getSciPtr()->popTXD();
            return const_cast<char*>(TX::getText(TX::SendRawDataMessage));
        }
    }

    return const_cast<char*>(TX::getText(TX::IncorrectEntry));
}


} /* namespace AME_SRC */
