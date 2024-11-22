/*
 * RequestFactory.h
 *
 *  Created on: 20.07.2022
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2022 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_REQUESTFACTORY_REQUESTFACTORY_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_REQUESTFACTORY_REQUESTFACTORY_H_

#include "../PlcCommand.h"

#include <cstdarg>
#include <cstdio>

namespace AME_SRC {

class RequestFactory {
 public:
    // static plcRequestNames requestName;
    RequestFactory();
    static PlcCommand& createRequest(plcRequestNames requestName, va_list args);
    static PlcCommand& createRequest(plcRequestNames requestName, int count, ...);
    static PlcCommand& createUserSetRequest(plcUserSetReqNames userSetCmdName, int count, ...);
    static uint8_t getConfirmPosition(LAYER layer, uint8_t cmdID);
    virtual ~RequestFactory();

 private:
    typedef struct {
        uint8_t *ptr;
        size_t size;
    } byteArray;
    static CHANNEL channelID;
    // G3-Commands
    static void fillTmpCmdWithInitReq(va_list args);
    static void getRecommendedParmForMode(CPX_MODE mode, RequestFactory::byteArray *recommendedParamSettings);
    static void fillTmpCmdWithSetConfigReq(va_list args);
    struct g3PramStruct {
        uint8_t coordinatorParameter[16];
        uint8_t peerParameter[12];
        uint8_t config[31];
        uint8_t GMK0[16];
        uint8_t defaultclientInfo[29];
    };
    static g3PramStruct g3Parameter;
    // ADP-Commands
    static void fillTmpCmdWithADPSend(va_list args);
    static void fillTmpCmdWithADPResetReq();
    static void fillTmpCmdWithADPSetReq(va_list args);
    static void prepareGetDataArgsForLayer(va_list args, LAYER layer, RequestFactory::byteArray *result);
    static void prepareSetDataArgsForLayer(va_list args, LAYER layer, RequestFactory::byteArray *result);
    static void fillTmpCmdWithADPGetReq(va_list args);
    static void fillTmpCmdWithADPNetworkStart(va_list args);
    static void fillTmpCmdWithADPRouteDiscoveryReq(va_list args);
    static void combineUint16ArgWithUint8Arg(va_list args, RequestFactory::byteArray *result);
    static void fillTmpCmdWithADPPathDiscoveryReq(va_list args);
    static void fillTmpCmdWithADPSearchNetwork(va_list args);
    static void fillTmpCmdWithADPJoin(va_list args);
    static void fillTmpCmdWithADPLeave();
    static void fillTmpCmdWithADPForceJoin(va_list args);
    // EAP-Commands
    static void fillTmpCmdWithEAPStartReq();
    static void fillTmpCmdWithEAPResetReq();
    static void fillTmpCmdWithEAPGetReq(va_list args);
    static void fillTmpCmdWithEAPSetReq(va_list args);
    static void fillTmpCmdWithEAPSetClientInfoReq(va_list args);
    static void fillTmpCmdWithEAPNetworkReq(va_list args);
    // UMAC-Commands
    static void fillTmpCmdWithMACGetReq(va_list args);
    static void fillTmpCmdWithMACSetReq(va_list args);
    // System Block
    static void fillTmpCmdWithGetSystemVersion();

    static PlcCommand tmpCommandContainer;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_G3PLCSYSTEM_REQUESTFACTORY_REQUESTFACTORY_H_
