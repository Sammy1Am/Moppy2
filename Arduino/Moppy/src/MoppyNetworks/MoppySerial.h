/*
 * MoppySerial.h
 *
 */

#ifndef SRC_MOPPYNETWORKS_MOPPYSERIAL_H_
#define SRC_MOPPYNETWORKS_MOPPYSERIAL_H_

#include <stdint.h>
#include "Arduino.h"
#include "../../MoppyConfig.h"
#include "MoppyNetwork.h"

#define MOPPY_BAUD_RATE 57600

typedef void (*handleSystemMessage)(uint8_t, uint8_t[]);
typedef void (*handleDeviceMessage)(uint8_t, uint8_t, uint8_t[]);

class MoppySerial {
  public:
    MoppySerial(handleSystemMessage, handleDeviceMessage);
    void begin(long baud = MOPPY_BAUD_RATE);
    void readMessages();
  private:
    handleSystemMessage systemHandler;
    handleDeviceMessage deviceHandler;
    uint8_t messagePos = 0; // Track current message read position
    uint8_t messageBuffer[259]; // Max message length for Moppy messages is 259
    uint8_t pongBytes[8] = {START_BYTE, 0x00, 0x00, 0x04, 0x81, DEVICE_ADDRESS, MIN_SUB_ADDRESS, MAX_SUB_ADDRESS};
    void sendPong();
};


#endif /* SRC_MOPPYNETWORKS_MOPPYSERIAL_H_ */
