/*
 * FloppyDrives.cpp
 *
 * Output for controlling floppy drives.  The _original_ Moppy instrument!
 */
#include "MoppyInstrument.h"
#include "DriveLights.h"
#include <math.h>

CRGB leds[NUM_LEDS];
const uint8_t driveToLEDMap[8] = {12,16,8,20,4,24,0,28};

void DriveLights::setup() {
  FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  FastLED.clear(true);
  startupShow();
}
//
//// Message Handlers
//

// Handles system messages (e.g. sequence start and stop)
void DriveLights::systemMessage(uint8_t command, uint8_t payload[]) {
  switch(command) {
      // NETBYTE_SYS_PING is handled by the network adapter directly
    case NETBYTE_SYS_RESET: // System reset
      FastLED.clear(true);
      break;
    case NETBYTE_SYS_START: // Sequence start
      // Nothing to do here yet
      break;
    case NETBYTE_SYS_STOP: // Sequence stop
      FastLED.clear(true);
      break;
  }
}

// Handles device-specific messages (e.g. playing notes)
void DriveLights::deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
  switch(command) {
    case NETBYTE_DEV_RESET: // Reset
      if (subAddress == 0x00) {
        FastLED.clear(true);
      } else {
        setDrive(subAddress, CRGB::Black);
      }
      break;
    case NETBYTE_DEV_NOTEON: // Note On
      // Set the current period to the new value to play it immediately
    	// Also set the originalPeriod in-case we pitch-bend
      setDrive(subAddress, getColor(subAddress, notePeriods[payload[0]]));
      break;
    case NETBYTE_DEV_NOTEOFF: // Note Off
      setDrive(subAddress, CRGB::Black);
      break;
    case NETBYTE_DEV_BENDPITCH: //Pitch bend
      //TODO
      // A value from -8192 to 8191 representing the pitch deflection
      int16_t bendDeflection = payload[0] << 8 | payload[1];

      // A whole octave of bend would double the frequency (halve the the period) of notes
      // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
      // //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
      // currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES*(bendDeflection/(float)8192));
      break;
  }
}

void DriveLights::setDrive(uint8_t driveNumber, CRGB newColor) {
  uint8_t startIndex = driveToLEDMap[driveNumber-1];
  for (uint8_t i = startIndex; i<startIndex+4;i++) {
    leds[i] = newColor;
  }
  FastLED.show();
}

void DriveLights::startupShow() {
  CRGB colors[3] = {CRGB::Red, CRGB::Green, CRGB::Blue};
  for (uint8_t c=0;c<3;c++){
    for (uint8_t i =1; i<=8;i++) {
      setDrive(i,colors[c]);
      delay(100);
      setDrive(i,CRGB::Black);
    }
  }
  
}

//TODO Add different color modes, and MIDI-message color assignments
CRGB DriveLights::getColor(uint8_t driveNumber, unsigned int notePeriod)
{
  return CHSV(int(128 * log(notePeriod / 478) / log(2)) % 255, 255, 255);
}
