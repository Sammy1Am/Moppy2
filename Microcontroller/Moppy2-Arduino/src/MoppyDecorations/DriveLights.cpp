/*
 * DriveLights.cpp
 *
 * Output for controlling decorative LEDs.
 */
#include "DriveLights.h"

// Still not sure why these can't all be defined in the header file... a TODO for sure.
CRGB leds[NUM_LEDS] = {0};
CHSV hsv_drives[NUM_DRIVES];
CHSV hsv_drives_background[NUM_DRIVES];
CHSV hsv_drives_target[NUM_DRIVES] = {RAINBOW_TARGET, RAINBOW_TARGET, RAINBOW_TARGET, RAINBOW_TARGET, RAINBOW_TARGET, RAINBOW_TARGET, RAINBOW_TARGET, RAINBOW_TARGET};
bool drives_fading[NUM_DRIVES];
uint8_t DRIVE_TO_LEDS_MAP[NUM_DRIVES] = {12, 16, 8, 20, 4, 24, 0, 28};

bool updateNeeded = false;

void DriveLights::setup() {
    FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, BRG>(leds, NUM_LEDS);
    FastLED.setBrightness(128);
    FastLED.clear(true);

    startupShow();

    lightsTicker = new Ticker(lightsTick, TICKER_RATE_MS, 0, MILLIS);
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

void DriveLights::copyToLEDS(uint8_t driveIndex) {
    for (uint8_t l = DRIVE_TO_LEDS_MAP[driveIndex]; l < DRIVE_TO_LEDS_MAP[driveIndex] + 4; l++) {
        leds[l] = hsv_drives[driveIndex];
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
        setDrive(subAddress - 1, CHSV(0, 0, 0));
    }
}
void DriveLights::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    setDrive(subAddress - 1, getColor(subAddress - 1, payload[0]));
}

void DriveLights::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    drives_fading[subAddress - 1] = 1; // Tell drive to fade (depending on mode)
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

void DriveLights::setDrive(uint8_t driveIndex, CHSV newColor) {
    hsv_drives[driveIndex] = newColor;
    copyToLEDS(driveIndex);
    updateNeeded = true;
}

void DriveLights::startupShow() {
    CHSV colors[3] = {RED_TARGET, BLUE_TARGET, GREEN_TARGET};
    for (uint8_t c = 0; c < 3; c++) {
        for (uint8_t i = 0; i < 8; i++) {
            setDrive(i, colors[c]);
            FastLED.show(); // Explicitly show because the Ticker isn't on yet.
            delay(100);
            setDrive(i, CHSV(0, 0, 0));
            FastLED.show(); // Explicitly show because the Ticker isn't on yet.
        }
    }
}

//TODO Add different color modes, and MIDI-message color assignments
CHSV DriveLights::getColor(uint8_t driveIndex, uint8_t noteNum) {
    if (hsv_drives_target[driveIndex] == RAINBOW_TARGET) {
        unsigned int notePeriod = NOTE_PERIODS[noteNum];
        return CHSV(int(128 * log(notePeriod / 478) / log(2)) % 255, 255, 255);
    } else {
        return hsv_drives_target[driveIndex];
    }
}

void DriveLights::fadeAllLights() {
    boolean updateCurrent = false;
    boolean updateAll = false;
    for (int l = 0; l < NUM_DRIVES; l++) {
        updateCurrent = false;

        if (drives_fading[l]) {
            if (hsv_drives[l].val < FADE_SPEED) {
                drives_fading[l] = false;
                hsv_drives[l].val = 0;
            } else {
                hsv_drives[l].val -= FADE_SPEED;
            }
            updateCurrent = true;
        }

        if (updateCurrent) {
            copyToLEDS(l);
            updateAll = true;
        }
    }

    updateNeeded |= updateAll;
}
