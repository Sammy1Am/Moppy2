/*
 * ShiftedFloppyDrives.cpp
 *
 * Output for controlling floppy drives via shifter(s).
 */
#include "ShiftedFloppyDrives.h"
#include "MoppyInstrument.h"
namespace instruments {

/*NOTE: The arrays below contain unused zero-indexes to avoid having to do extra
 * math to shift the 1-based subAddresses to 0-based indexes here.  Unlike the previous
 * version of Moppy, we *will* be doing math to calculate which drive maps to which pin,
 * so there are as many values as drives (plus the extra zero-index)
 */

/*An array of maximum track positions for each floppy drive.  3.5" Floppies have
 80 tracks, 5.25" have 50.  These should be doubled, because each tick is now
 half a position (use 158 and 98).
 NOTE: Index zero of this array controls the "resetAll" function, and should be the
 same as the largest value in this array
 */
unsigned int ShiftedFloppyDrives::MAX_POSITION[] = {158, 158, 158, 158, 158, 158, 158, 158};

//Array to track the current position of each floppy head.
unsigned int ShiftedFloppyDrives::currentPosition[] = {0, 0, 0, 0, 0, 0, 0, 0};

/*Array to keep track of state of each pin.  Even bits track the control-pins for toggle purposes.  Odd bits
 track direction-pins (LOW = forward, HIGH=reverse)
 */
uint8_t ShiftedFloppyDrives::currentState[] = {0, 0};

// Current period assigned to each drive.  0 = off.  Each period is two-ticks (as defined by
// TIMER_RESOLUTION in MoppyInstrument.h) long.
unsigned int ShiftedFloppyDrives::currentPeriod[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Tracks the current tick-count for each drive (see ShiftedFloppyDrives::tick() below)
unsigned int ShiftedFloppyDrives::currentTick[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// The period originally set by incoming messages (prior to any modifications from pitch-bending)
unsigned int ShiftedFloppyDrives::originalPeriod[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void ShiftedFloppyDrives::setup() {

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    SPI.begin();

    // With all pins setup, let's do a first run reset
    resetAll();
    delay(500); // Wait a half second for safety

      // Setup timer to handle interrupts for floppy driving
      MoppyTimer::initialize(TIMER_RESOLUTION, tick);

      // If MoppyConfig wants a startup sound, play the startupSound on the
      // first drive.
      if (PLAY_STARTUP_SOUND) {
          startupSound(FIRST_DRIVE);
          delay(500);
          resetAll();
    }
}

// Play startup sound to confirm drive functionality
void ShiftedFloppyDrives::startupSound(byte driveNum) {
    unsigned int chargeNotes[] = {
        noteDoubleTicks[31],
        noteDoubleTicks[36],
        noteDoubleTicks[38],
        noteDoubleTicks[43],
        0};
    byte i = 0;
    unsigned long lastRun = 0;
    while (i < 5) {
        if (millis() - 200 > lastRun) {
            lastRun = millis();
            currentPeriod[driveNum] = chargeNotes[i++];
        }
    }
}

//
//// Message Handlers
//

void ShiftedFloppyDrives::sys_reset() {
    resetAll();
}

void ShiftedFloppyDrives::sys_sequenceStop() {
    haltAllDrives();
}

void ShiftedFloppyDrives::dev_reset(uint8_t subAddress) {
    if (subAddress == 0x00) {
        resetAll();
    } else {
        reset(subAddress);
    }
}

void ShiftedFloppyDrives::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    if (payload[0] <= MAX_FLOPPY_NOTE) {
        currentPeriod[subAddress] = originalPeriod[subAddress] = noteDoubleTicks[payload[0]];
    }
};
void ShiftedFloppyDrives::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    currentPeriod[subAddress] = originalPeriod[subAddress] = 0;
};
void ShiftedFloppyDrives::dev_bendPitch(uint8_t subAddress, uint8_t payload[]) {
    // A value from -8192 to 8191 representing the pitch deflection
    int16_t bendDeflection = payload[0] << 8 | payload[1];

    // A whole octave of bend would double the frequency (halve the the period) of notes
    // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
    //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
    currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES * (bendDeflection / (float)8192));
};

//
//// Floppy driving functions
//

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
 */
void ShiftedFloppyDrives::tick() {
    bool shiftNeeded = false;
    /*
   If there is a period set for control pin 2, count the number of
   ticks that pass, and toggle the pin if the current period is reached.
   */
    if (currentPeriod[1] > 0) {
        currentTick[1]++;
        if (currentTick[1] >= currentPeriod[1]) {
            togglePin(0, 0, 1); // Drive 1 is on pins 2 and 3
            shiftNeeded = true;
            currentTick[1] = 0;
        }
    }
    if (currentPeriod[2] > 0) {
        currentTick[2]++;
        if (currentTick[2] >= currentPeriod[2]) {
            togglePin(1, 2, 3);
            shiftNeeded = true;
            currentTick[2] = 0;
        }
    }
    if (currentPeriod[3] > 0) {
        currentTick[3]++;
        if (currentTick[3] >= currentPeriod[3]) {
            togglePin(2, 4, 5);
            shiftNeeded = true;
            currentTick[3] = 0;
        }
    }
    if (currentPeriod[4] > 0) {
        currentTick[4]++;
        if (currentTick[4] >= currentPeriod[4]) {
            togglePin(3, 6, 7);
            shiftNeeded = true;
            currentTick[4] = 0;
        }
    }
    if (currentPeriod[5] > 0) {
        currentTick[5]++;
        if (currentTick[5] >= currentPeriod[5]) {
            togglePin(4, 8, 9);
            shiftNeeded = true;
            currentTick[5] = 0;
        }
    }
    if (currentPeriod[6] > 0) {
        currentTick[6]++;
        if (currentTick[6] >= currentPeriod[6]) {
            togglePin(5, 10, 11);
            shiftNeeded = true;
            currentTick[6] = 0;
        }
    }
    if (currentPeriod[7] > 0) {
        currentTick[7]++;
        if (currentTick[7] >= currentPeriod[7]) {
            togglePin(6, 12, 13);
            shiftNeeded = true;
            currentTick[7] = 0;
        }
    }
    if (currentPeriod[8] > 0) {
        currentTick[8]++;
        if (currentTick[8] >= currentPeriod[8]) {
            togglePin(7, 14, 15);
            shiftNeeded = true;
            currentTick[8] = 0;
        }
    }
    if (shiftNeeded) {shiftBits();}
}

void ShiftedFloppyDrives::togglePin(byte driveIndex, byte pinIndex, byte directionPinIndex) {
    int byteIndex = driveIndex / 4; // Index in currentState to find the bit

    //Switch directions if end has been reached
    if (currentPosition[driveIndex] >= MAX_POSITION[driveIndex]) {
        bitWrite(currentState[byteIndex], directionPinIndex - (8 * byteIndex), 1);
    } else if (currentPosition[driveIndex] <= 0) {
        bitWrite(currentState[byteIndex], directionPinIndex - (8 * byteIndex), 0);
    }

    //Update currentPosition
    if (bitRead(currentState[byteIndex], directionPinIndex - (8 * byteIndex))){
        currentPosition[driveIndex]--;
    } else {
        currentPosition[driveIndex]++;
    }

    currentState[byteIndex] ^= (1 << (pinIndex - (8 * byteIndex)));
}

void ShiftedFloppyDrives::shiftBits() {
    digitalWrite(LATCH_PIN, LOW); // Lower LATCH

    SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));

    for (int s = 1; s >= 0; s--) {
      SPI.transfer(currentState[s]);
    }

    SPI.endTransaction();
    
    digitalWrite(LATCH_PIN, HIGH);
}

//
//// UTILITY FUNCTIONS
//

// Immediately stops all drives
void ShiftedFloppyDrives::haltAllDrives() {
    for (byte d = FIRST_DRIVE; d <= LAST_DRIVE; d++) {
        currentPeriod[d] = 0;
    }
}

//For a given floppy number, runs the read-head all the way back to 0
void ShiftedFloppyDrives::reset(byte driveNum) {
    currentPeriod[driveNum] = 0; // Stop note

    byte stepPin = driveNum * 2;
    digitalWrite(stepPin + 1, HIGH);                               // Go in reverse
    for (unsigned int s = 0; s < MAX_POSITION[driveNum]; s += 2) { //Half max because we're stepping directly (no toggle)
        digitalWrite(stepPin, HIGH);
        digitalWrite(stepPin, LOW);
        delay(5);
    }
    currentPosition[driveNum] = 0; // We're reset.
    currentState[stepPin] = LOW;
    digitalWrite(stepPin + 1, LOW);
    currentState[stepPin + 1] = LOW; // Ready to go forward.
}

// Resets all the drives simultaneously
void ShiftedFloppyDrives::resetAll() {

    // Stop all drives and set to reverse
    for (byte d = FIRST_DRIVE; d <= LAST_DRIVE; d++) {
        currentPeriod[d] = 0;
    }
    currentState[0] = B10101010;
    currentState[1] = B10101010;
    shiftBits();

    // Reset all drives together
    for (unsigned int s = 0; s < MAX_POSITION[0]; s += 2) { //Half max because we're stepping directly (no toggle); grab max from index 0
        currentState[0] = B10101010;
        currentState[1] = B10101010;
        shiftBits();
        currentState[0] = B11111111;
        currentState[1] = B11111111;
        shiftBits();
        delay(5);
    }

    // Return tracking to ready state
    currentState[0] = B00000000;
    currentState[1] = B00000000;
    for (byte d = FIRST_DRIVE; d <= LAST_DRIVE; d++) {
        currentPosition[d] = 0; // We're reset.
    }
}
} // namespace instruments