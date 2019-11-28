/*
 * DriveLights.cpp
 *
 * Output for controlling decorative LEDs.
 */
#include "DriveLights.h"

CRGB leds[NUM_LEDS];
CHSV hsv_leds[NUM_LEDS];
const uint8_t driveToLEDMap[8] = {12, 16, 8, 20, 4, 24, 0, 28};
bool updateNeeded = false;
const uint8_t FADE_SPEED = 10;

void DriveLights::setup() {
    FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, BRG>(leds, NUM_LEDS);
    FastLED.setBrightness(128);
    FastLED.clear(true);

    startupShow();

    lightsTicker = new Ticker(lightsTick, 10, 0, MILLIS);
    lightsTicker->start();
}

void DriveLights::decoLoop() {
    lightsTicker->update();
}

void DriveLights::lightsTick() {
    fadeAllLights();

    if (updateNeeded) {
        updateNeeded = false;
        FastLED.show();
    }
}

//
//// Message Handlers
//
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
        setDriveRGB(subAddress, CRGB::Black);
    }
}
void DriveLights::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    setDrive(subAddress, getColor(subAddress, decorationNotePeriods[payload[0]]));
}
void DriveLights::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    //setDrive(subAddress, CRGB::Black);
    fadeDrive(subAddress);
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

void DriveLights::setDriveRGB(uint8_t driveNumber, CRGB newColor) {
    uint8_t startIndex = driveToLEDMap[driveNumber - 1];
    for (uint8_t i = startIndex; i < startIndex + 4; i++) {
        leds[i] = newColor;
    }
    updateNeeded = true;
}

void DriveLights::setDrive(uint8_t driveNumber, CHSV newColor) {
    uint8_t startIndex = driveToLEDMap[driveNumber - 1];
    for (uint8_t i = startIndex; i < startIndex + 4; i++) {
        hsv_leds[i] = newColor;
        leds[i] = newColor;
    }
    updateNeeded = true;
}

void DriveLights::fadeDrive(uint8_t driveNumber) {
    uint8_t startIndex = driveToLEDMap[driveNumber - 1];
    for (uint8_t i = startIndex; i < startIndex + 4; i++) {
        hsv_leds[i].val = 254;
    }
}

    void DriveLights::startupShow() {
    CRGB colors[3] = {CRGB::Red, CRGB::Green, CRGB::Blue};
    for (uint8_t c = 0; c < 3; c++) {
        for (uint8_t i = 1; i <= 8; i++) {
            setDriveRGB(i, colors[c]);
            FastLED.show(); // Explicitly show because the Ticker isn't on yet.
            delay(100);
            setDriveRGB(i, CRGB::Black);
            FastLED.show(); // Explicitly show because the Ticker isn't on yet.
        }
    }
}

//TODO Add different color modes, and MIDI-message color assignments
CHSV DriveLights::getColor(uint8_t driveNumber, unsigned int notePeriod) {
    return CHSV(int(128 * log(notePeriod / 478) / log(2)) % 255, 255, 255);
}

void DriveLights::fadeAllLights() {
    boolean updateCurrent = false;
    boolean updateAll = false;
    for (int l = 0; l < NUM_LEDS; l++) {
        updateCurrent = false;

        if (hsv_leds[l].val == 0) {
            continue;
        } else if (hsv_leds[l].val < FADE_SPEED) {
            hsv_leds[l].val = 0;
            updateCurrent = true;
        } else if (hsv_leds[l].val < 255) {
            hsv_leds[l].val -= FADE_SPEED;
            updateCurrent = true;
        }

        if (updateCurrent) {
            leds[l] = hsv_leds[l];
            updateAll = true;
        }
    }

    updateNeeded |= updateAll;
}
