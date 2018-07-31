/*
 * FloppyDrives.cpp
 *
 * Output for controlling floppy drives.  The _original_ Moppy instrument!
 *
 * Now configured for STM32 chips such as the popular STM32f103C8T6 which this was tested on
 */
#include "MoppyInstrument.h"
#include "FloppyDrives.h"


// First drive being used for floppies, and the last drive.  Used for calculating
// step and direction pins.
const byte FIRST_DRIVE = 1;
const byte LAST_DRIVE = 8;  // This sketch can handle only up to 8 drives (the max for the STM32F103C8T6)

//array of pin definitions to avoid the hard coded use of arduino uno pin names
const uint8 devicePins[18] = {0, 0, PB10, PB11, PB9, PB8, PA9, PA8, PB7, PB6, PB15, PB14, PB4, PB3, PB13, PB12, PA15, PA10};

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
unsigned int FloppyDrives::MAX_POSITION[] = {158,158,158,158,158,158,158,158,158};

//Array to track the current position of each floppy head.
unsigned int FloppyDrives::currentPosition[] = {0,0,0,0,0,0,0,0,0};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse
 */
int FloppyDrives::currentState[] = {0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

// Current period assigned to each drive.  0 = off.  Each period is two-ticks (as defined by
// TIMER_RESOLUTION in MoppyInstrument.h) long.
unsigned int FloppyDrives::currentPeriod[] = {0,0,0,0,0,0,0,0,0};

// Tracks the current tick-count for each drive (see FloppyDrives::tick() below)
unsigned int FloppyDrives::currentTick[] = {0,0,0,0,0,0,0,0,0};

// The period originally set by incoming messages (prior to any modifications from pitch-bending)
unsigned int FloppyDrives::originalPeriod[] = {0,0,0,0,0,0,0,0,0};

// Declaration of the hardware timer which controls the interrupt for the tick function
// This is for the STM32
HardwareTimer driveTimer(2);

void FloppyDrives::setup() {

  // Prepare pins, avoiding ones not 5V tolerant
  pinMode(PB10, OUTPUT); // Step control 1
  pinMode(PB11, OUTPUT); // Direction 1
  pinMode(PB9, OUTPUT); // Step control 2
  pinMode(PB8, OUTPUT); // Direction 2
  pinMode(PA9, OUTPUT); // Step control 3
  pinMode(PA8, OUTPUT); // Direction 3
  pinMode(PB7, OUTPUT); // Step control 4
  pinMode(PB6, OUTPUT); // Direction 4
  pinMode(PB15, OUTPUT); // Step control 5
  pinMode(PB14, OUTPUT); // Direction 5
  pinMode(PB4, OUTPUT); // Step control 6
  pinMode(PB3, OUTPUT); // Direction 6
  pinMode(PB13, OUTPUT); // Step control 7
  pinMode(PB12, OUTPUT); // Direction 7
  pinMode(PA15, OUTPUT); // Step control 8
  pinMode(PA10, OUTPUT); // Direction 8


  // With all pins setup, let's do a first run reset
  resetAll();
  delay(500); // Wait a half second for safety

  // Setup timer to handle interrupts for floppy driving - now in STM32 for Arduino fashion
  driveTimer.pause();
  driveTimer.setPeriod(TIMER_RESOLUTION);
  driveTimer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  driveTimer.setCompare(TIMER_CH1, 1);
  driveTimer.attachCompare1Interrupt(tick);
  driveTimer.refresh();
  driveTimer.resume();
  
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

// Handles system messages (e.g. sequence start and stop)
void FloppyDrives::systemMessage(uint8_t command, uint8_t payload[]) {
  switch(command) {
      // NETBYTE_SYS_PING is handled by the network adapter directly
    case NETBYTE_SYS_RESET: // System reset
      resetAll();
      break;
    case NETBYTE_SYS_START: // Sequence start
      // Nothing to do here yet
      break;
    case NETBYTE_SYS_STOP: // Sequence stop
      haltAllDrives();
      break;
  }
}

// Handles device-specific messages (e.g. playing notes)
void FloppyDrives::deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
  switch(command) {
    case NETBYTE_DEV_RESET: // Reset
      if (subAddress == 0x00) {
        resetAll();
      } else {
        reset(subAddress);
      }
      break;
    case NETBYTE_DEV_NOTEON: // Note On
      // Set the current period to the new value to play it immediately
    	// Also set the originalPeriod in-case we pitch-bend
      if (payload[0] <= MAX_FLOPPY_NOTE) {
        currentPeriod[subAddress] = originalPeriod[subAddress] = noteDoubleTicks[payload[0]];
      }
      break;
    case NETBYTE_DEV_NOTEOFF: // Note Off
      currentPeriod[subAddress] = originalPeriod[subAddress] = 0;
      break;
    case NETBYTE_DEV_BENDPITCH: //Pitch bend
      // A value from -8192 to 8191 representing the pitch deflection
      int16_t bendDeflection = payload[0] << 8 | payload[1];

      // A whole octave of bend would double the frequency (halve the the period) of notes
      // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
      //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
      currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES*(bendDeflection/(float)8192));
      break;
  }
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
      togglePin(1);
      currentTick[1]=0;
    }
  }
  if (currentPeriod[2]>0){
    currentTick[2]++;
    if (currentTick[2] >= currentPeriod[2]){
      togglePin(2);
      currentTick[2]=0;
    }
  }
  if (currentPeriod[3]>0){
    currentTick[3]++;
    if (currentTick[3] >= currentPeriod[3]){
      togglePin(3);
      currentTick[3]=0;
    }
  }
  if (currentPeriod[4]>0){
    currentTick[4]++;
    if (currentTick[4] >= currentPeriod[4]){
      togglePin(4);
      currentTick[4]=0;
    }
  }
  if (currentPeriod[5]>0){
    currentTick[5]++;
    if (currentTick[5] >= currentPeriod[5]){
      togglePin(5);
      currentTick[5]=0;
    }
  }
  if (currentPeriod[6]>0){
    currentTick[6]++;
    if (currentTick[6] >= currentPeriod[6]){
      togglePin(6);
      currentTick[6]=0;
    }
  }
  if (currentPeriod[7]>0){
    currentTick[7]++;
    if (currentTick[7] >= currentPeriod[7]){
      togglePin(7);
      currentTick[7]=0;
    }
  }
  if (currentPeriod[8]>0){
    currentTick[8]++;
    if (currentTick[8] >= currentPeriod[8]){
      togglePin(8);
      currentTick[8]=0;
    }
  }
}

void FloppyDrives::togglePin(byte driveNum) {

  //Switch directions if end has been reached
  if (currentPosition[driveNum] >= MAX_POSITION[driveNum]) {
    currentState[(driveNum*2)+1] = HIGH;
    digitalWrite(devicePins[(driveNum*2)+1],HIGH);
  }
  else if (currentPosition[driveNum] <= 0) {
    currentState[(driveNum*2)+1] = LOW;
    digitalWrite(devicePins[(driveNum*2)+1],LOW);
  }

  //Update currentPosition
  if (currentState[(driveNum*2)+1] == HIGH){
    currentPosition[driveNum]--;
  }
  else {
    currentPosition[driveNum]++;
  }

  //Pulse the control pin
  digitalWrite(devicePins[(driveNum*2)],currentState[driveNum*2]);
  currentState[driveNum*2] = ~currentState[driveNum*2];
}


//
//// UTILITY FUNCTIONS
//

//Not used now, but good for debugging... (flipped around for STM32 as low is on)
void FloppyDrives::blinkLED(){
  digitalWrite(PC13, LOW); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(PC13, HIGH);
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
  digitalWrite(devicePins[stepPin+1],HIGH); // Go in reverse
  for (unsigned int s=0;s<MAX_POSITION[driveNum];s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(devicePins[stepPin],HIGH);
    digitalWrite(devicePins[stepPin],LOW);
    delay(5);
  }
  currentPosition[driveNum] = 0; // We're reset.
  currentState[stepPin] = LOW;
  digitalWrite(devicePins[stepPin+1],LOW);
  currentState[stepPin+1] = LOW; // Ready to go forward.
}

// Resets all the drives simultaneously
void FloppyDrives::resetAll()
{

  // Stop all drives and set to reverse
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    byte stepPin = d * 2;
    currentPeriod[d] = 0;
    digitalWrite(devicePins[stepPin+1],HIGH);
  }

  // Reset all drives together
  for (unsigned int s=0;s<MAX_POSITION[0];s+=2){ //Half max because we're stepping directly (no toggle); grab max from index 0
    for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
      byte stepPin = d * 2;
      digitalWrite(devicePins[stepPin],HIGH);
      digitalWrite(devicePins[stepPin],LOW);
    }
    delay(5);
  }

  // Return tracking to ready state
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    byte stepPin = d * 2;
    currentPosition[d] = 0; // We're reset.
    currentState[stepPin] = LOW;
    digitalWrite(devicePins[stepPin+1],LOW);
    currentState[stepPin+1] = LOW; // Ready to go forward.
  }
}
