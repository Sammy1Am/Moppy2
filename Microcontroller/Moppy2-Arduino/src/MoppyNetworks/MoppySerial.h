/*
 * MoppySerial.h
 *
 */

#ifndef SRC_MOPPYNETWORKS_MOPPYSERIAL_H_
#define SRC_MOPPYNETWORKS_MOPPYSERIAL_H_

#include <stdint.h>
#include "Arduino.h"
#include "../MoppyConfig.h"
#include "../MoppyMessageConsumer.h"
#include "MoppyNetwork.h"

#define MOPPY_BAUD_RATE 57600

class MoppySerial {
  public:
      MoppySerial(MoppyMessageConsumer *messageConsumer);
      void begin();
      void readMessages();
  private:
    MoppyMessageConsumer *targetConsumer;
    uint8_t messagePos = 0; // Track current message read position
    uint8_t messageBuffer[259]; // Max message length for Moppy messages is 259
    uint8_t pongBytes[8] = {START_BYTE, 0x00, 0x00, 0x04, 0x81, DEVICE_ADDRESS, MIN_SUB_ADDRESS, MAX_SUB_ADDRESS};
    void sendPong();
};


#endif /* SRC_MOPPYNETWORKS_MOPPYSERIAL_H_ */