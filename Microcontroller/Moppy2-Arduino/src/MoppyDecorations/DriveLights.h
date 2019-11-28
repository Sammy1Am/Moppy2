/*
 * DriveLights.h
 *
 */

#ifndef SRC_MOPPYDECORATIONS_DRIVELIGHTS_H_
#define SRC_MOPPYDECORATIONS_DRIVELIGHTS_H_

#include "../MoppyConfig.h"
#include "MoppyDecoration.h"
#include <Arduino.h>
#include <Ticker.h>
#include <math.h>

#include <FastLED.h>
#define NUM_LEDS 32
#define DATA_PIN 18
#define CLOCK_PIN 19

class DriveLights : public MoppyDecoration {
public:
  void setup() override;
  void decoLoop() override;

  protected:
  //static CRGB leds[NUM_LEDS];
      Ticker *lightsTicker;

      void sys_sequenceStop() override;
      void sys_reset() override;

      void dev_reset(uint8_t subAddress) override;
      void dev_noteOn(uint8_t subAddress, uint8_t payload[]) override;
      void dev_noteOff(uint8_t subAddress, uint8_t payload[]) override;
      void dev_bendPitch(uint8_t subAddress, uint8_t payload[]) override;

      static void lightsTick();
      static void setDriveRGB(uint8_t driveNumber, CRGB newColor);
      static void setDrive(uint8_t driveNumber, CHSV newColor);
      static void fadeDrive(uint8_t driveNumber);
      static CHSV getColor(uint8_t driveNumber, unsigned int notePeriod);
      static void startupShow();
  private:
      static void fadeAllLights();
};

#endif /* SRC_MOPPYDECORATIONS_DRIVELIGHTS_H_ */
