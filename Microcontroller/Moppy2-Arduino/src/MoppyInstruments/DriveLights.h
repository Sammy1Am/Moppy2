/*
 * DriveLights.h
 *
 */

#ifndef SRC_MOPPYINSTRUMENTS_DRIVELIGHTS_H_
#define SRC_MOPPYINSTRUMENTS_DRIVELIGHTS_H_

#include <Arduino.h>
#include <TimerOne.h>
#include "MoppyInstrument.h"
#include "../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

#include <FastLED.h>
#define NUM_LEDS 32
#define DATA_PIN 18
#define CLOCK_PIN 19

class DriveLights : public MoppyInstrument {
public:
  void setup();
  void systemMessage(uint8_t command, uint8_t payload[]);
  void deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]);
protected:
  //static CRGB leds[NUM_LEDS];

  static void setDrive(uint8_t driveNumber, CRGB newColor);
  static CRGB getColor(uint8_t driveNumber, unsigned int notePeriod);
  static void startupShow();
};



#endif /* SRC_MOPPYINSTRUMENTS_DRIVELIGHTS_H_ */
