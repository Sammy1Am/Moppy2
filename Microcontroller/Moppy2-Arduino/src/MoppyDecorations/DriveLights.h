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
#define NUM_DRIVES 8
#define NUM_LEDS (4*NUM_DRIVES)
#define DATA_PIN 18
#define CLOCK_PIN 19

const CHSV RED_TARGET{0, 255, 255};
const CHSV GREEN_TARGET{96, 255, 255};
const CHSV BLUE_TARGET{160, 255, 255};
const CHSV DIM_TARGET{0, 0, 32};
const CHSV BLACK_TARGET{0, 0, 0};
const CHSV RAINBOW_TARGET{1, 0, 0}; // Set to target rainbow colors

// The period of notes in microseconds
const unsigned int NOTE_PERIODS[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30578, 28861, 27242, 25713, 24270, 22909, 21622, 20409, 19263, 18182, 17161, 16198, //C1 - B1
    15289, 14436, 13621, 12856, 12135, 11454, 10811, 10205, 9632, 9091, 8581, 8099,     //C2 - B2
    7645, 7218, 6811, 6428, 6068, 5727, 5406, 5103, 4816, 4546, 4291, 4050,             //C3 - B3
    3823, 3609, 3406, 3214, 3034, 2864, 2703, 2552, 2408, 2273, 2146, 2025,             //C4 - B4
    1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012,             //C5 - B5
    956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506,                         //C6 - B6
    478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253,                         //C7 - B7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

class DriveLights : public MoppyDecoration {
public:
    void setup() override;
    void decoLoop() override;

protected:
    Ticker *lightsTicker;

    void sys_sequenceStop() override;
    void sys_reset() override;

    void dev_reset(uint8_t subAddress) override;
    void dev_noteOn(uint8_t subAddress, uint8_t payload[]) override;
    void dev_noteOff(uint8_t subAddress, uint8_t payload[]) override;
    void dev_bendPitch(uint8_t subAddress, uint8_t payload[]) override;

    static void lightsTick();
    static void setDrive(uint8_t driveIndex, CHSV newColor);
    static CHSV getColor(uint8_t driveIndex, uint8_t noteNum);
    static void startupShow();

private:
    static void fadeAllLights();
    static void copyToLEDS(uint8_t driveIndex);
    static void resetToBackground();

    static const uint8_t FADE_SPEED = 20;
    static const uint8_t TICKER_RATE_MS = 20;
};

#endif /* SRC_MOPPYDECORATIONS_DRIVELIGHTS_H_ */
