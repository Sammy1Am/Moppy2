/*
 * MoppyMidi.h
 *
 */

#ifndef SRC_MOPPYNETWORKS_MOPPYSERIAL_H_
#define SRC_MOPPYNETWORKS_MOPPYSERIAL_H_

#include <stdint.h>
#include "Arduino.h"
#include "../MoppyConfig.h"
#include "../MoppyMessageConsumer.h"
#include "MoppyNetwork.h"

#define MOPPY_BAUD_RATE 31250
#define STEREO false
#define ONLY3BYTE false

class MoppyMidi {
  public:
    MoppyMidi(MoppyMessageConsumer *messageConsumer);
    void begin();
    void readMessages();
  private:
      MoppyMessageConsumer *targetConsumer;
};


#endif /* SRC_MOPPYNETWORKS_MOPPYSERIAL_H_ */
