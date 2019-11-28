/*
 * CompoundConsumer.h
 *
 */

#ifndef SRC_MOPPYINSTRUMENTS_COMPOUNDCONSUMER_H_
#define SRC_MOPPYINSTRUMENTS_COMPOUNDCONSUMER_H_

#include <Arduino.h>
#include "MoppyInstrument.h"
#include "../MoppyConfig.h"
#include "../MoppyMessageConsumer.h"

class CompoundConsumer : public MoppyMessageConsumer {
private:
    MoppyMessageConsumer *consumerA;
    MoppyMessageConsumer *consumerB;

public:
    void handleSystemMessage(uint8_t command, uint8_t payload[]) override;
    void handleDeviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) override;
    CompoundConsumer(MoppyMessageConsumer *a, MoppyMessageConsumer *b) {
        consumerA = a;
        consumerB = b;
  }
};

#endif /* SRC_MOPPYINSTRUMENTS_COMPOUNDCONSUMER_H_ */
