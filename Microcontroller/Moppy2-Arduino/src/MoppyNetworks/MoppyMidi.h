/*
 * MoppyMidi.h
 *
 */

#ifndef SRC_MOPPYNETWORKS_MOPPYSERIAL_H_
#define SRC_MOPPYNETWORKS_MOPPYSERIAL_H_

#include <stdint.h>
#include "Arduino.h"
#include "../MoppyConfig.h"
#include "MoppyNetwork.h"

#define MOPPY_BAUD_RATE 31250
#define STEREO false
#define ONLY3BYTE false
typedef void (*handleDeviceMessage)(uint8_t, uint8_t, uint8_t[]);

class MoppyMidi {
  public:
    MoppyMidi(handleDeviceMessage);
    void begin(long baud = MOPPY_BAUD_RATE);
    void readMessages();
  private:
    handleDeviceMessage deviceHandler;
};


#endif /* SRC_MOPPYNETWORKS_MOPPYSERIAL_H_ */
