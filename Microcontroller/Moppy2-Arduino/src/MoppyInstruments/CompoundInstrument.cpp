/*
 * CompoundInstrument.cpp
 *
 * Output for controlling multiple instruments.
 * //TODO Would be nice if this could support arbitrary numbers of insturments instead of just two
 */
#include "MoppyInstrument.h"
#include "CompoundInstrument.h"

void CompoundInstrument::setup() {
  instrumentA->setup();
  instrumentB->setup();
}

//
//// Message Handlers
//

// Handles system messages (e.g. sequence start and stop)
void CompoundInstrument::systemMessage(uint8_t command, uint8_t payload[]) {
  instrumentA->systemMessage(command, payload);
  instrumentB->systemMessage(command, payload);
}

// Handles device-specific messages (e.g. playing notes)
void CompoundInstrument::deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
  instrumentA->deviceMessage(subAddress, command, payload);
  instrumentB->deviceMessage(subAddress, command, payload);
}