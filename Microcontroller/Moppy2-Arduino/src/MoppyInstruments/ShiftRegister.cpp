/*
 * ShiftRegister.cpp
 *
 * Output for controlling a single device that uses shift registers, one
 * bit for each note.  E.g. Xylophones or drums
 */
#include "ShiftRegister.h"

#include "MoppyInstrument.h"

namespace instruments {
// Define pins for connection to shift registers
#define DATA_PIN 2
#define SHIFT_CLOCK_PIN 3
#define LATCH_PIN 4
#define MASTER_CLEAR_PIN 5
#define OUTPUT_ENABLE_PIN 6

// First and last supported notes (any notes outside this range will be ignored, first note will be
// indexed as zero for shifting
#define NUM_NOTES 24
const uint8_t FIRST_NOTE = 79;
const uint8_t LAST_NOTE = FIRST_NOTE + (NUM_NOTES-1);

#define SHIFT_TIMER_RESOLUTION 1000 //Microsecond resolution for starting and ending notes

// The velocity of the incoming notes will adjust the pulse length from MIN_PULSE_TICKS to MIN_PULSE_TICKS + PULSE_TICKS_RANGE
#define MIN_PULSE_TICKS 10 // Minimum length of "on" pulse for each bit in ticks (e.g. RESOLUTION * PULSE_TICKS microseconds)
#define PULSE_TICKS_RANGE 60 // Maximum number of ticks to add to MIN_PULSE_TICKS for maximum velocity

#define SHIFT_DATA_BYTES 3
uint8_t ShiftRegister::shiftData[SHIFT_DATA_BYTES]; // 8*3 = 24
uint8_t ShiftRegister::activeTicksLeft[NUM_NOTES];

boolean ShiftRegister::shouldShift = false; // When true, a shift will occur during the next tick

void ShiftRegister::setup() {

  // Prepare pins
  pinMode(DATA_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Don't enable these unless you plan to use them!
  //pinMode(MASTER_CLEAR_PIN, OUTPUT);
  //pinMode(OUTPUT_ENABLE_PIN, OUTPUT);

  pinMode(13, OUTPUT); // Built-in LED for blinking


  // With all pins setup, let's do a first run reset
  zeroOutputs();
  delay(500); // Wait a half second for safety

  // Setup timer to handle interrupts for floppy driving
  MoppyTimer::initialize(SHIFT_TIMER_RESOLUTION, tick);
}


//
//// Message Handlers
//

void ShiftRegister::sys_reset() {
    zeroOutputs();
}

void ShiftRegister::sys_sequenceStop() {
    zeroOutputs();
}

void ShiftRegister::dev_reset(uint8_t subAddress) {
    zeroOutputs(); // SubAddress unimportant here since we only have one
}

void ShiftRegister::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    if (payload[0] >= FIRST_NOTE && payload[0] <= LAST_NOTE) {
        outputOn(payload[0] - FIRST_NOTE);
        activeTicksLeft[payload[0] - FIRST_NOTE] = MIN_PULSE_TICKS + ((payload[1] * PULSE_TICKS_RANGE) / 127);
        shouldShift = true;
    }
}

//
//// Floppy driving functions
//

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
 */
void ShiftRegister::tick()
{
  for (byte n = 0; n < NUM_NOTES; n++) {
    if (activeTicksLeft[n] > 0) {
      if (--activeTicksLeft[n] == 0) {
        outputOff(n);
        shouldShift = true;
      }
    }
  }

  if (shouldShift) {
    shouldShift = false;
    shiftAllData();
  }
}

////
// UTILITY FUNCTIONS
////

//Not used now, but good for debugging...
void blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW);
}


////
// Shifting and bitsetting functions
////

void ShiftRegister::shiftAllData()
{
  digitalWrite(LATCH_PIN, LOW);
  for (int i=SHIFT_DATA_BYTES-1;i>=0;i--){
    shiftOut(DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, shiftData[i]);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

void ShiftRegister::outputOn(byte outputNum){
  bitSet(shiftData[outputNum/8],outputNum%8);
}

void ShiftRegister::outputOff(byte outputNum){
  bitClear(shiftData[outputNum/8],outputNum%8);
}

void ShiftRegister::zeroOutputs() {
  for (byte i=0;i<SHIFT_DATA_BYTES;i++){
    shiftData[i] = 0;
  }
  shiftAllData();
}
} // namespace instruments
