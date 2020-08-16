/*
 * FloppyDrives.cpp
 *
 * Output for controlling floppy drives.  The _original_ Moppy instrument!
 */
#include "MoppyInstrument.h"
#include "FloppyDrives.h"

namespace instruments {
// First drive being used for floppies, and the last drive.  Used for calculating
// step and direction pins.
const byte FIRST_DRIVE = 1;
const byte LAST_DRIVE = 8;  // This sketch can handle only up to 9 drives (the max for Arduino Uno)

// Maximum note number to attempt to play on floppy drives.  It's possible higher notes may work,
// but they may also cause instability.
const byte MAX_FLOPPY_NOTE = 71;

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
unsigned int FloppyDrives::MAX_POSITION[] = {158,158,158,158,158,158,158,158,158,158};

//Array to track the current position of each floppy head.
unsigned int FloppyDrives::currentPosition[] = {0,0,0,0,0,0,0,0,0,0};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse
 */
int FloppyDrives::currentState[] = {0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

// Current period assigned to each drive.  0 = off.  Each period is two-ticks (as defined by
// TIMER_RESOLUTION in MoppyInstrument.h) long.
unsigned int FloppyDrives::currentPeriod[] = {0,0,0,0,0,0,0,0,0,0};

// Tracks the current tick-count for each drive (see FloppyDrives::tick() below)
unsigned int FloppyDrives::currentTick[] = {0,0,0,0,0,0,0,0,0,0};

// The period originally set by incoming messages (prior to any modifications from pitch-bending)
unsigned int FloppyDrives::originalPeriod[] = {0,0,0,0,0,0,0,0,0,0};

void FloppyDrives::setup() {

  // Prepare pins (0 and 1 are reserved for Serial communications)
  pinMode(2, OUTPUT); // Step control 1
  pinMode(3, OUTPUT); // Direction 1
  pinMode(4, OUTPUT); // Step control 2
  pinMode(5, OUTPUT); // Direction 2
  pinMode(6, OUTPUT); // Step control 3
  pinMode(7, OUTPUT); // Direction 3
  pinMode(8, OUTPUT); // Step control 4
  pinMode(9, OUTPUT); // Direction 4
  pinMode(10, OUTPUT); // Step control 5
  pinMode(11, OUTPUT); // Direction 5
  pinMode(12, OUTPUT); // Step control 6
  pinMode(13, OUTPUT); // Direction 6
  pinMode(14, OUTPUT); // Step control 7
  pinMode(15, OUTPUT); // Direction 7
  pinMode(16, OUTPUT); // Step control 8
  pinMode(17, OUTPUT); // Direction 8
  pinMode(18, OUTPUT); // Direction 9
  pinMode(19, OUTPUT); // Step control 9


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
void FloppyDrives::startupSound(byte driveNum) {
  unsigned int chargeNotes[] = {
      noteDoubleTicks[31],
      noteDoubleTicks[36],
      noteDoubleTicks[38],
      noteDoubleTicks[43],
      0
  };
  byte i = 0;
  unsigned long lastRun = 0;
  while(i < 5) {
    if (millis() - 200 > lastRun) {
      lastRun = millis();
      currentPeriod[driveNum] = chargeNotes[i++];
    }
  }
}

//
//// Message Handlers
//

void FloppyDrives::sys_reset() {
    resetAll();
}

void FloppyDrives::sys_sequenceStop() {
    haltAllDrives();
}

void FloppyDrives::dev_reset(uint8_t subAddress) {
    if (subAddress == 0x00) {
        resetAll();
    } else {
        reset(subAddress);
    }
}

void FloppyDrives::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    if (payload[0] <= MAX_FLOPPY_NOTE) {
        currentPeriod[subAddress] = originalPeriod[subAddress] = noteDoubleTicks[payload[0]];
    }
}

void FloppyDrives::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    currentPeriod[subAddress] = originalPeriod[subAddress] = 0;
}

void FloppyDrives::dev_bendPitch(uint8_t subAddress, uint8_t payload[]) {
    // A value from -8192 to 8191 representing the pitch deflection
    int16_t bendDeflection = payload[0] << 8 | payload[1];

    // A whole octave of bend would double the frequency (halve the the period) of notes
    // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
    //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
    currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES * (bendDeflection / (float)8192));
}

//
//// Floppy driving functions
//

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
 */
void FloppyDrives::tick()
{
  /*
   If there is a period set for control pin 2, count the number of
   ticks that pass, and toggle the pin if the current period is reached.
   */
  if (currentPeriod[1]>0){
    currentTick[1]++;
    if (currentTick[1] >= currentPeriod[1]){
      togglePin(1,2,3); // Drive 1 is on pins 2 and 3
      currentTick[1]=0;
    }
  }
  if (currentPeriod[2]>0){
    currentTick[2]++;
    if (currentTick[2] >= currentPeriod[2]){
      togglePin(2,4,5);
      currentTick[2]=0;
    }
  }
  if (currentPeriod[3]>0){
    currentTick[3]++;
    if (currentTick[3] >= currentPeriod[3]){
      togglePin(3,6,7);
      currentTick[3]=0;
    }
  }
  if (currentPeriod[4]>0){
    currentTick[4]++;
    if (currentTick[4] >= currentPeriod[4]){
      togglePin(4,8,9);
      currentTick[4]=0;
    }
  }
  if (currentPeriod[5]>0){
    currentTick[5]++;
    if (currentTick[5] >= currentPeriod[5]){
      togglePin(5,10,11);
      currentTick[5]=0;
    }
  }
  if (currentPeriod[6]>0){
    currentTick[6]++;
    if (currentTick[6] >= currentPeriod[6]){
      togglePin(6,12,13);
      currentTick[6]=0;
    }
  }
  if (currentPeriod[7]>0){
    currentTick[7]++;
    if (currentTick[7] >= currentPeriod[7]){
      togglePin(7,14,15);
      currentTick[7]=0;
    }
  }
  if (currentPeriod[8]>0){
    currentTick[8]++;
    if (currentTick[8] >= currentPeriod[8]){
      togglePin(8,16,17);
      currentTick[8]=0;
    }
  }
  if (currentPeriod[9]>0){
    currentTick[9]++;
    if (currentTick[9] >= currentPeriod[9]){
      togglePin(9,18,19);
      currentTick[9]=0;
    }
  }
}

void FloppyDrives::togglePin(byte driveNum, byte pin, byte direction_pin) {

  //Switch directions if end has been reached
  if (currentPosition[driveNum] >= MAX_POSITION[driveNum]) {
    currentState[direction_pin] = HIGH;
    digitalWrite(direction_pin,HIGH);
  }
  else if (currentPosition[driveNum] <= 0) {
    currentState[direction_pin] = LOW;
    digitalWrite(direction_pin,LOW);
  }

  //Update currentPosition
  if (currentState[direction_pin] == HIGH){
    currentPosition[driveNum]--;
  }
  else {
    currentPosition[driveNum]++;
  }

  //Pulse the control pin
  digitalWrite(pin,currentState[pin]);
  currentState[pin] = ~currentState[pin];
}


//
//// UTILITY FUNCTIONS
//

//Not used now, but good for debugging...
void FloppyDrives::blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW);
}

// Immediately stops all drives
void FloppyDrives::haltAllDrives() {
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    currentPeriod[d] = 0;
  }
}

//For a given floppy number, runs the read-head all the way back to 0
void FloppyDrives::reset(byte driveNum)
{
  currentPeriod[driveNum] = 0; // Stop note

  byte stepPin = driveNum * 2;
  digitalWrite(stepPin+1,HIGH); // Go in reverse
  for (unsigned int s=0;s<MAX_POSITION[driveNum];s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(stepPin,HIGH);
    digitalWrite(stepPin,LOW);
    delay(5);
  }
  currentPosition[driveNum] = 0; // We're reset.
  currentState[stepPin] = LOW;
  digitalWrite(stepPin+1,LOW);
  currentState[stepPin+1] = LOW; // Ready to go forward.
}

// Resets all the drives simultaneously
void FloppyDrives::resetAll()
{

  // Stop all drives and set to reverse
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    byte stepPin = d * 2;
    currentPeriod[d] = 0;
    digitalWrite(stepPin+1,HIGH);
  }

  // Reset all drives together
  for (unsigned int s=0;s<MAX_POSITION[0];s+=2){ //Half max because we're stepping directly (no toggle); grab max from index 0
    for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
      byte stepPin = d * 2;
      digitalWrite(stepPin,HIGH);
      digitalWrite(stepPin,LOW);
    }
    delay(5);
  }

  // Return tracking to ready state
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    byte stepPin = d * 2;
    currentPosition[d] = 0; // We're reset.
    currentState[stepPin] = LOW;
    digitalWrite(stepPin+1,LOW);
    currentState[stepPin+1] = LOW; // Ready to go forward.
  }
}
} // namespace instruments
