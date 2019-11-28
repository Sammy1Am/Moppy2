/*
 * CompoundInstrument.cpp
 *
 * Output for controlling multiple instruments.
 * //TODO Would be nice if this could support arbitrary numbers of insturments instead of just two
 */
#include "MoppyInstrument.h"
#include "CompoundConsumer.h"

//
//// Message Handlers
//

// Handles system messages (e.g. sequence start and stop)
void CompoundConsumer::handleSystemMessage(uint8_t command, uint8_t payload[]) {
    consumerA->handleSystemMessage(command, payload);
    consumerB->handleSystemMessage(command, payload);
}

// Handles device-specific messages (e.g. playing notes)
void CompoundConsumer::handleDeviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
    consumerA->handleDeviceMessage(subAddress, command, payload);
    consumerB->handleDeviceMessage(subAddress, command, payload);
}