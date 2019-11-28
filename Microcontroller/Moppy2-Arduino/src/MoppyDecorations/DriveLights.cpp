/*
 * DriveLights.cpp
 *
 * Output for controlling decorative LEDs.
 */
#include "DriveLights.h"
#include "MoppyInstrument.h"
#include <math.h>

CRGB leds[NUM_LEDS];
const uint8_t driveToLEDMap[8] = {12, 16, 8, 20, 4, 24, 0, 28};
bool updateNeeded = false;

void DriveLights::setup() {
    FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, BRG>(leds, NUM_LEDS);
    FastLED.setBrightness(128);
    FastLED.clear(true);

    //Ticker lightsTicker(lightsTick, 100);
    //lightsTicker.start();

    startupShow();
}

void DriveLights::lightsTick() {
  if (updateNeeded) {
      updateNeeded = false;
      FastLED.show();
  }
}

//
//// Message Handlers
//
void DriveLights::sys_sequenceStart() {
    FastLED.clear(true);
}
void DriveLights::sys_sequenceStop() {
    FastLED.clear(true);
}
void DriveLights::sys_reset() {
    FastLED.clear(true);
}

void DriveLights::dev_reset(uint8_t subAddress) {
    if (subAddress == 0x00) {
        FastLED.clear(true);
    } else {
        setDrive(subAddress, CRGB::Black);
    }
}
void DriveLights::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    setDrive(subAddress, getColor(subAddress, notePeriods[payload[0]]));
}
void DriveLights::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    setDrive(subAddress, CRGB::Black);
}
void DriveLights::dev_bendPitch(uint8_t subAddress, uint8_t payload[]) {
    //TODO
    // A value from -8192 to 8191 representing the pitch deflection
    int16_t bendDeflection = payload[0] << 8 | payload[1];

    // A whole octave of bend would double the frequency (halve the the period) of notes
    // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
    // //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
    // currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES*(bendDeflection/(float)8192));
}

void DriveLights::setDrive(uint8_t driveNumber, CRGB newColor) {
    uint8_t startIndex = driveToLEDMap[driveNumber - 1];
    for (uint8_t i = startIndex; i < startIndex + 4; i++) {
        leds[i] = newColor;
    }
    updateNeeded = true;
    FastLED.show();
}

void DriveLights::startupShow() {
    CRGB colors[3] = {CRGB::Red, CRGB::Green, CRGB::Blue};
    for (uint8_t c = 0; c < 3; c++) {
        for (uint8_t i = 1; i <= 8; i++) {
            setDrive(i, colors[c]);
            delay(100);
            setDrive(i, CRGB::Black);
        }
    }
}

//TODO Add different color modes, and MIDI-message color assignments
CRGB DriveLights::getColor(uint8_t driveNumber, unsigned int notePeriod) {
    return CHSV(int(128 * log(notePeriod / 478) / log(2)) % 255, 255, 255);
}
