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
#define DATA_PIN 4
#define CLOCK_PIN 5

#define NETBYTE_DEV_SETTARGETCOLOR 0x61
#define NETBYTE_DEV_SETBGCOLOR 0x62

const CHSV RED_TARGET{0, 255, 255};
const CHSV GREEN_TARGET{96, 255, 255};
const CHSV BLUE_TARGET{160, 255, 255};
const CHSV DIM_TARGET{0, 0, 16};
const CHSV BLACK_TARGET{0, 0, 0};
const CHSV RAINBOW_TARGET{1, 0, 0}; // Set to target rainbow colors

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
    void deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]);

private:
    static void lightsTick();
    static void setDrive(uint8_t driveIndex, CHSV newColor);
    static CHSV getColor(uint8_t driveIndex, uint8_t noteNum);
    static void startupShow();
    static void fadeAllLights();
    static void copyToLEDS(uint8_t driveIndex);
    static void fadeToBackground();

    static void setTargetColor(uint8_t driveIndex, CHSV newColor);
    static void setBackgroundColor(uint8_t driveIndex, CHSV newColor);

    static CHSV originalSetColor[NUM_DRIVES];
    static const uint8_t FADE_SPEED = 20;
    static const uint8_t TICKER_RATE_MS = 20;
};

#endif /* SRC_MOPPYDECORATIONS_DRIVELIGHTS_H_ */
