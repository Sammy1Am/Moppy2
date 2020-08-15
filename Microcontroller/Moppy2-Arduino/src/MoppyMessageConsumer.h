/*
 * MoppyMessageConsumer.h
 * Base class for any objects that consume MoppyMessages (with some helpful functions for common messages).
 */

#ifndef MOPPY_SRC_MOPPYMESSAGECONSUMER_H_
#define MOPPY_SRC_MOPPYMESSAGECONSUMER_H_

#include "MoppyNetworks/MoppyNetwork.h"
#include <Arduino.h>

class MoppyMessageConsumer {
public:
    virtual void handleSystemMessage(uint8_t command, uint8_t payload[]) {
        switch (command) {
        // NETBYTE_SYS_PING is handled by the network adapter directly
        case NETBYTE_SYS_START: // Sequence start
            sys_sequenceStart();
            break;
        case NETBYTE_SYS_STOP: // Sequence stop
            sys_sequenceStop();
            break;
        case NETBYTE_SYS_RESET: // System reset
            sys_reset();
            break;
        default:
            systemMessage(command, payload); // Fallback on a generic handler in case there's an implementation-specific message
            break;
        }
    };

    virtual void handleDeviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
        switch (command) {
        case NETBYTE_DEV_RESET: // Reset
            if (subAddress == 0x00) {
                sys_reset();
            } else {
                dev_reset(subAddress);
            }
            break;
        case NETBYTE_DEV_NOTEON: // Note On
            dev_noteOn(subAddress, payload);
            break;
        case NETBYTE_DEV_NOTEOFF: // Note Off
            dev_noteOff(subAddress, payload);
            break;
        case NETBYTE_DEV_BENDPITCH: //Pitch bend
            dev_bendPitch(subAddress, payload);
            break;
        default:
            deviceMessage(subAddress, command, payload);
            break;
        };
    };

protected:
    virtual void sys_sequenceStart(){};
    virtual void sys_sequenceStop(){};
    virtual void sys_reset(){};
    virtual void systemMessage(uint8_t command, uint8_t payload[]){};

    virtual void dev_reset(uint8_t subAddress){};
    virtual void dev_noteOn(uint8_t subAddress, uint8_t payload[]){};
    virtual void dev_noteOff(uint8_t subAddress, uint8_t payload[]){};
    virtual void dev_bendPitch(uint8_t subAddress, uint8_t payload[]){};
    virtual void deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]){}; //TODO Probably should include a payloadLength on all of these...
};

#endif /* MOPPY_SRC_MOPPYMESSAGECONSUMER_H_ */