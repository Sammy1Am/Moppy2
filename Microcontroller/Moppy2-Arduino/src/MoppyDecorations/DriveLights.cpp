/*
 * DriveLights.cpp
 *
 * Output for controlling decorative LEDs.
 */
#include "DriveLights.h"

// Still not sure why these can't all be defined in the header file... a TODO for sure.
CRGB leds[NUM_LEDS];
CHSV hsv_drives[NUM_DRIVES];
CHSV hsv_drives_background[NUM_DRIVES];
CHSV hsv_drives_target[NUM_DRIVES] = {
    RAINBOW_TARGET, 
    RAINBOW_TARGET, 
    RAINBOW_TARGET, 
    RAINBOW_TARGET, 
    RAINBOW_TARGET, 
    RAINBOW_TARGET, 
    RAINBOW_TARGET, 
    RAINBOW_TARGET};

bool drives_fading[NUM_DRIVES];
uint8_t DRIVE_TO_LEDS_MAP[NUM_DRIVES] = {12, 16, 8, 20, 4, 24, 0, 28};

CHSV DriveLights::originalSetColor[NUM_DRIVES];

bool updateNeeded = false;

void DriveLights::setup() {
    FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, BRG>(leds, NUM_LEDS);
    FastLED.setBrightness(128);
    FastLED.clear(true);
    FastLED.clear(true); // Do this twice to eliminate some weird glowing

    if (PLAY_STARTUP_SOUND) {
        startupShow();
    }

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

void DriveLights::fadeToBackground() {
    for (uint8_t d = 0; d < NUM_DRIVES; d++) {
        drives_fading[d] = true;
    }
    updateNeeded = true;
}

//
//// Message Handlers
//
void DriveLights::sys_sequenceStop() {
    fadeToBackground();
}
void DriveLights::sys_reset() {
    FastLED.clear(true);
    // Reset all targeted colors
    for (int t = 0; t < NUM_DRIVES;t++){
        hsv_drives_target[t] = RAINBOW_TARGET;
        hsv_drives_background[t] = BLACK_TARGET;
    }
}

void DriveLights::dev_reset(uint8_t subAddress) {
    if (subAddress == 0x00) {
        FastLED.clear(true);
    } else {
        setDrive(subAddress - 1, hsv_drives_background[subAddress - 1]);
    }
}
void DriveLights::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    CHSV newColor = getColor(subAddress - 1, payload[0]);
    setDrive(subAddress - 1, newColor);
    originalSetColor[subAddress - 1] = newColor;
}

void DriveLights::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    drives_fading[subAddress - 1] = true; // Tell drive to fade (depending on mode)
}
void DriveLights::dev_bendPitch(uint8_t subAddress, uint8_t payload[]) {
    if (originalSetColor[subAddress - 1].sat==0){
        return; // If there's no color, don't worry about bending
    }
    // A value from -8192 to 8191 representing the pitch deflection
    int16_t bendDeflection = payload[0] << 8 | payload[1];

    // A whole octave of bend would add 127 "degrees" (out of 255) to the hue
    // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection

    uint8_t newHue = originalSetColor[subAddress - 1].hue + (255 * (BEND_OCTAVES * (bendDeflection / (float)8192)));
    setDrive(subAddress - 1, CHSV(newHue, originalSetColor[subAddress - 1].sat, originalSetColor[subAddress - 1].val));
}

void DriveLights::deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
    switch (command) {
    case NETBYTE_DEV_SETTARGETCOLOR:
        setTargetColor(subAddress - 1, CHSV(payload[0]*2, payload[1]*2, payload[2]*2)); // MIDI bytes only go to 127, so * 2
        break;
    case NETBYTE_DEV_SETBGCOLOR:
        setBackgroundColor(subAddress - 1, CHSV(payload[0]*2, payload[1]*2, payload[2]*2)); // MIDI bytes only go to 127, so * 2
        break;
        }
}

void DriveLights::setTargetColor(uint8_t driveIndex, CHSV newColor) {
    hsv_drives_target[driveIndex] = newColor;
}

// Assign new drive background color, and fade to it immediately
void DriveLights::setBackgroundColor(uint8_t driveIndex, CHSV newColor) {
    hsv_drives_background[driveIndex] = newColor;
    drives_fading[driveIndex] = true;
}

void DriveLights::setDrive(uint8_t driveIndex, CHSV newColor) {
    drives_fading[driveIndex] = false; // Don't fade if we're setting to a new color!
    hsv_drives[driveIndex] = newColor;
    copyToLEDS(driveIndex);
    updateNeeded = true;
}

void DriveLights::startupShow() {
    CHSV colors[3] = {RED_TARGET, GREEN_TARGET, BLUE_TARGET};
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

CHSV DriveLights::getColor(uint8_t driveIndex, uint8_t noteNum) {
    if (hsv_drives_target[driveIndex] == RAINBOW_TARGET) {
        return CHSV((noteNum%24)*10.6, 255, 255); // Color-wheel over 24 notes (2 octaves)
    } 
    // Targets with 2-sat and 2-val will have their sat scaled instead of hue
    else if (hsv_drives_target[driveIndex].sat == 2 && hsv_drives_target[driveIndex].val == 2) {
        return CHSV(hsv_drives_target[driveIndex].hue, ((noteNum % 12) * 16)+79, 255); // Vary saturation over 24 notes (2 octaves)
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
            CHSV targetColor = hsv_drives_background[l];
            int hueDelta = 0;
            int satDelta = 0;
            // Only worry about hue and saturation if the target is going to be visible at all
            if (targetColor.val > 0) {

                hueDelta = hsv_drives[l].hue - targetColor.hue;

                if (hueDelta > FADE_SPEED) {
                    hsv_drives[l].hue -= FADE_SPEED;
                } else if (hueDelta < -FADE_SPEED) {
                    hsv_drives[l].hue += FADE_SPEED;
                } else {
                    hsv_drives[l].hue = targetColor.hue;
                    hueDelta = 0;
                }

                satDelta = hsv_drives[l].sat - targetColor.sat;

                if (satDelta > FADE_SPEED) {
                    hsv_drives[l].sat -= FADE_SPEED;
                } else if (satDelta < -FADE_SPEED) {
                    hsv_drives[l].sat += FADE_SPEED;
                } else {
                    hsv_drives[l].sat = targetColor.sat;
                    satDelta = 0;
                }
            }

            int valDelta = hsv_drives[l].val - targetColor.val;

            if (valDelta > FADE_SPEED) {
                hsv_drives[l].val -= FADE_SPEED;
            } else if (valDelta < -FADE_SPEED) {
                hsv_drives[l].val += FADE_SPEED;
            } else {
                hsv_drives[l].val = targetColor.val;
                valDelta = 0;

                // If we just got through fading to black, we're done; set the color properties
                if (targetColor.val == 0) {
                    hsv_drives[l].hue = targetColor.hue;
                    hsv_drives[l].sat = targetColor.sat;
                }
            }

            
            updateCurrent = true;

            if (hueDelta == 0 && satDelta == 0 && valDelta == 0) {
                // We made it!
                drives_fading[l] = false;
            }
        }

        if (updateCurrent) {
            copyToLEDS(l);
            updateAll = true;
        }
    }

    updateNeeded |= updateAll;
}
