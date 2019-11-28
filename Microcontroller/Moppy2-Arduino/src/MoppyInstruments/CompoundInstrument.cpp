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
void CompoundInstrument::handleSystemMessage(uint8_t command, uint8_t payload[]) {
  instrumentA->handleSystemMessage(command, payload);
  instrumentB->handleSystemMessage(command, payload);
}

// Handles device-specific messages (e.g. playing notes)
void CompoundInstrument::handleDeviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
    instrumentA->handleDeviceMessage(subAddress, command, payload);
    instrumentB->handleDeviceMessage(subAddress, command, payload);
}