/*
 * AliveData.h
 *
 *  Created on: Aug 4, 2023
 *      Author: T. Hirsch, AME
 *  Copyright (c) 2023 Andreas Müller electronic GmbH (AME)
 */

#ifndef SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_ALIVEDATA_H_
#define SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_ALIVEDATA_H_

#include <cstdint>

namespace AME_SRC {

class AliveData {
 public:
    typedef enum {
        Alive_Request,     // Active Alive request
        Alive_Confirm,     // Active Alive acknowledge
        Alive_Indication,  // Passive Alive message
        Alive_Unknown,     // Placeholder / error case
        Alive_RoundTrip,   // Finalization of an alive message
    } aliveType;

    enum eBitSizes {Trips = 8, Nodes = 8, Retries = 8 };  // ToDo(AME): 3,10,8

    typedef union {
        uint32_t all;
        struct {
            uint32_t roundTripID :Trips;
            uint32_t passedNodes :8;    // Peers, nodes on round trip
            uint32_t retries :8;
            uint32_t padding :8;        // spare space
        } bit;
        uint8_t byte[4];
    } dataField_t;

    static const  char alivePrefix[4];

    struct chainMessage {
        const char *prefix;
        dataField_t dataField;
        uint8_t memberID;
    };

    enum Direction { incoming, outgoing };

    AliveData(const enum Direction inout, uint16_t inoutGroup, uint8_t memberID);
    void incrementPassedNodes();
    virtual ~AliveData();
    bool isValid() {
        return true;
    }

    uint16_t getIncomingGroup() {
        return incomingGroup_;
    }
    void setIncomingGroup(uint16_t group) {
        incomingGroup_ = group;
    }

    uint16_t getOutgoingGroup() {
        return outgoingGroup_;
    }
    void setOutgoingGroup(uint16_t group) {
        outgoingGroup_ = group;
    }

    uint8_t getMemberID() {
        return data_.memberID;
    }
    void setMemberID(uint8_t id) {
        data_.memberID = id;
    }

    aliveType getStatus() const {
        return status_;
    }

    void setStatus(aliveType status) {
        status_ = status;
    }

    uint32_t getDataField() const {
        return data_.dataField.all;
    }
    void setDataField(uint32_t set) {
        uint8_t tmp = 0;
        for (int i = 0; i < 4; i++) {
            tmp = set & 0xFF;
            data_.dataField.byte[i] = tmp;
            set = set >> 8;
        }
    }
    uint8_t getRoundTripID() const {
        return data_.dataField.bit.roundTripID;
    }
    void setRoundTripID(uint16_t set) {
        data_.dataField.bit.roundTripID = set & ((1 << Trips) - 1);
    }
    uint16_t getPassedNodes() const {
        return data_.dataField.bit.passedNodes;
    }
    void setPassedNodes(uint16_t set) {
        data_.dataField.bit.passedNodes = set & ((1 << Nodes) - 1);
    }

    static dataField_t getDataFieldWithIncNodes(dataField_t data) {
        uint32_t temp = data.bit.passedNodes + 1;
        data.bit.passedNodes = temp & ((1 << Nodes) - 1);
        return data;
    }

    uint8_t getRetries() const {
        return data_.dataField.bit.retries;
    }
    void setRetries(uint16_t set) {
        data_.dataField.bit.retries = set & ((1 << Retries) - 1);
    }
    void incRetries() {
        data_.dataField.bit.retries = (uint8_t) (data_.dataField.bit.retries + 1);
    }

 private:
    uint16_t incomingGroup_;    // uint16_t srcIP_;
    uint16_t outgoingGroup_;    // uint16_t targetIP_;
    chainMessage data_;
    aliveType status_;
};

}  // namespace AME_SRC

#endif  // SRC_AME_PROJECT_PLCCOMMUNICATION_ALIVEHANDLING_MESSAGEFORWARDING_ALIVEDATA_H_
