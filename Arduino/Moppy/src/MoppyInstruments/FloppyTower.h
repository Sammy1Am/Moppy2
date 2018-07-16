/*
 * FloppyTower.h
 * Connect all your drives normally!
 * The rules have changed! One note for all of the floppy drives
 * in this Arduino.
 * The amount of fdd that plays varies with the velocity and the Exponential Decay
 * Designed for big setups
 */

#ifndef SRC_MOPPYINSTRUMENTS_FLOPPYTOWER_H_
#define SRC_MOPPYINSTRUMENTS_FLOPPYTOWER_H_

#include <Arduino.h>
#include <TimerOne.h>
#include "../../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

class FloppyTower {
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
  static int velocity;
  static int FIRST_DRIVE;
  static int LAST_DRIVE;
  static int MAX_FLOPPY_NOTE;
  static unsigned int lastVelocity;

  static void resetAll();
  static void togglePin(byte driveNum, byte pin, byte direction_pin);
  static void haltAllDrives();
  static void reset(byte driveNum);
  static void tick();
  static void blinkLED();
  static void startupSound(byte driveNum);
};



#endif /* SRC_MOPPYINSTRUMENTS_FLOPPYTOWER_H_ */
