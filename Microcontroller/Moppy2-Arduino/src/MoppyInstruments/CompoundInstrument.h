/*
 * CompoundInstrument.h
 *
 */

#ifndef SRC_MOPPYINSTRUMENTS_COMPOUNDINSTRUMENT_H_
#define SRC_MOPPYINSTRUMENTS_COMPOUNDINSTRUMENT_H_

#include <Arduino.h>
#include "MoppyInstrument.h"
#include "../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

class CompoundInstrument : public MoppyInstrument {
  private:
    MoppyInstrument* instrumentA;
    MoppyInstrument* instrumentB;

  public:
    void setup();
    void handleSystemMessage(uint8_t command, uint8_t payload[]);
    void handleDeviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]);
    CompoundInstrument(MoppyInstrument *a, MoppyInstrument *b)
    {
      instrumentA = a;
      instrumentB = b;
  }
};


#endif /* SRC_MOPPYINSTRUMENTS_COMPOUNDINSTRUMENT_H_ */
