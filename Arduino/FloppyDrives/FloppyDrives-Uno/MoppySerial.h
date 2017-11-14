#ifndef MoppySerial_h
#define MoppySerial_h

#include <stdint.h>
#include "Arduino.h"

#define DEVICE_ADDRESS 0x01
#define MIN_SUB_ADDRESS 1
#define MAX_SUB_ADDRESS 8

#define MOPPY_BAUD_RATE 57600

//TODO: These should their own.. namespace?  file?
#define START_BYTE 0x4d
#define SYSTEM_ADDRESS 0x00

typedef void (*handleSystemMessage)(uint8_t, uint8_t[]);
typedef void (*handleDeviceMessage)(uint8_t, uint8_t, uint8_t[]);

class MoppySerial {
  public:
    MoppySerial(handleSystemMessage, handleDeviceMessage);
    void begin(long baud = MOPPY_BAUD_RATE);
    void readMessages();
    void sendPong();
  private:
    handleSystemMessage systemHandler;
    handleDeviceMessage deviceHandler;
    uint8_t messagePos = 0; // Track current message read position
    uint8_t messageBuffer[259]; // Max message length for Moppy messages is 259
    uint8_t pongBytes[7] = {START_BYTE, 0x00, 0x04, 0x81, DEVICE_ADDRESS, MIN_SUB_ADDRESS, MAX_SUB_ADDRESS};
};

#endif
