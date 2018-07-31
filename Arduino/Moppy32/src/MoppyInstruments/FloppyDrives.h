/*
 * FloppyDrives.h
 *
 */

#ifndef SRC_MOPPYINSTRUMENTS_FLOPPYDRIVES_H_
#define SRC_MOPPYINSTRUMENTS_FLOPPYDRIVES_H_

#include <Arduino.h>
#include "../../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

class FloppyDrives {
public:
  static void setup();
  static void systemMessage(uint8_t command, uint8_t payload[]);
  static void deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]);
protected:
  static unsigned int MAX_POSITION[];
  static unsigned int currentPosition[];
  static int currentState[];
  static unsigned int currentPeriod[];
  static unsigned int currentTick[];
  static unsigned int originalPeriod[];

  static void resetAll();
  static void togglePin(byte driveNum);
  static void haltAllDrives();
  static void reset(byte driveNum);
  static void tick();
  static void blinkLED();
  static void startupSound(byte driveNum);
};



#endif /* SRC_MOPPYINSTRUMENTS_FLOPPYDRIVES_H_ */
