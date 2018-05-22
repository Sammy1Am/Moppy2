/*
 * L298N.h
 *
 */

#ifndef SRC_MOPPYINSTRUMENTS_L298N_H_
#define SRC_MOPPYINSTRUMENTS_L298N_H_

#include <Arduino.h>
#include <TimerOne.h>
#include "../../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

class L298N {
public:
  static void setup();
  static void systemMessage(uint8_t command, uint8_t payload[]);
  static void deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]);
protected:
  static int FIRST_BRIDGE;
  static int LAST_BRIDGE;
  static unsigned int MAX_POSITION[];
  static unsigned int currentPosition[];
  static int currentStep[];
  static int currentDir[];
  static unsigned int currentPeriod[];
  static unsigned int currentTick[];
  static unsigned int originalPeriod[];
  static void resetAll();
  static void step(byte bridgeNum, byte pin1, byte pin2, byte pin3, byte pin4);
  static void haltAllDrives();
  static void reset(byte bridgeNum);
  static void tick();
  static void blinkLED();
  static void startupSound(byte bridgeNum);
  static void L298Nvariables();
};

#endif /* SRC_MOPPYINSTRUMENTS_L298N_H_ */
