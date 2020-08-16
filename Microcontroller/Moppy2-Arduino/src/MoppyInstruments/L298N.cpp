/*
 * L298N.cpp
 * @author : Sammy1Am, modified for the L298N by Lothean
 * Output for controlling L298N stepper driver.
 */
#include "MoppyInstrument.h"
#include "L298N.h"

namespace instruments {
// Used to keep track of what to do for the next step according to the table of bipolar stepper motors.
int L298N::currentStep[] = {0,0,0,0,0}; 

// First and last bridge
int L298N::FIRST_BRIDGE = 1;
int L298N::LAST_BRIDGE = 4;  // This sketch can handle only up to 4 bridges (the max for Arduino Uno)

/*NOTE: The arrays below contain unused zero-indexes to avoid having to do extra
 * math to shift the 1-based subAddresses to 0-based indexes here.  Unlike the previous
 * version of Moppy, we *will* be doing math to calculate which drive maps to which pin,
 * so there are as many values as drives (plus the extra zero-index)
 */

 /*An array of maximum steps for all your drivers. No need to double the steps here since
 it's not working as a floppy drive.
 */
unsigned int L298N::MAX_POSITION[] = {0,3000,3000,3000,3000};

//Array to track the current position of each floppy head.
unsigned int L298N::currentPosition[] = {0,0,0,0,0};

/*Array to keep track of the position of each driver.  0 = forward, 1 = reverse
 */
int L298N::currentDir[] = {0,0,0,0,0};

// Current period assigned to each drive.  0 = off.  Each period is two-ticks (as defined by
// TIMER_RESOLUTION in MoppyInstrument.h) long.
unsigned int L298N::currentPeriod[] = {0,0,0,0,0};

// Tracks the current tick-count for each drive (see FloppyDrives::tick() below)
unsigned int L298N::currentTick[] = {0,0,0,0,0};

// The period originally set by incoming messages (prior to any modifications from pitch-bending)
unsigned int L298N::originalPeriod[] = {0,0,0,0,0};

void L298N::setup() {

  // Prepare pins (0 and 1 are reserved for Serial communications)
  pinMode(2, OUTPUT); // IN1 for bridge 1
  pinMode(3, OUTPUT); // IN2 for bridge 1
  pinMode(4, OUTPUT); // IN3 for bridge 1
  pinMode(5, OUTPUT); // IN4 for bridge 1
  pinMode(6, OUTPUT); // IN1 for bridge 2
  pinMode(7, OUTPUT); // IN2 for bridge 2
  pinMode(8, OUTPUT); // IN3 for bridge 2
  pinMode(9, OUTPUT); // IN4 for bridge 2
  pinMode(10, OUTPUT); // IN1 for bridge 3
  pinMode(11, OUTPUT); // IN2 for bridge 3
  pinMode(12, OUTPUT); // IN3 for bridge 3
  pinMode(13, OUTPUT); // IN4 for bridge 3
  pinMode(14, OUTPUT); // IN1 for bridge 4
  pinMode(15, OUTPUT); // IN2 for bridge 4
  pinMode(16, OUTPUT); // IN3 for bridge 4
  pinMode(17, OUTPUT); // IN4 for bridge 4


  // With all pins setup, let's do a first run reset
  resetAll();
  delay(500); // Wait a half second for safety

  // Setup timer to handle interrupts for driving the bridges
  MoppyTimer::initialize(TIMER_RESOLUTION, tick);

  // If MoppyConfig wants a startup sound, play the startupSound on the
  // first drive.
  if (PLAY_STARTUP_SOUND) {
    startupSound(FIRST_BRIDGE);
    delay(500);
    resetAll();
  }
}

// Play startup sound to confirm drive functionality
void L298N::startupSound(byte driveNum) {
  unsigned int chargeNotes[] = {
      noteTicks[31],
      noteTicks[36],
      noteTicks[38],
      noteTicks[43],
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

void L298N::sys_reset() {
    resetAll();
}

void L298N::sys_sequenceStop() {
    haltAllDrives();
}

void L298N::dev_reset(uint8_t subAddress) {
    if (subAddress == 0x00) {
        resetAll();
    } else {
        reset(subAddress);
    }
}

void L298N::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    currentPeriod[subAddress] = originalPeriod[subAddress] = noteTicks[payload[0]];
}

void L298N::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    currentPeriod[subAddress] = originalPeriod[subAddress] = 0;
};

void L298N::dev_bendPitch(uint8_t subAddress, uint8_t payload[]) {
    // A value from -8192 to 8191 representing the pitch deflection
    int16_t bendDeflection = payload[0] << 8 | payload[1];

    // A whole octave of bend would double the frequency (halve the the period) of notes
    // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
    //L298N::currentPeriod[subAddress] = L298N::originalPeriod[subAddress] / 1.4;
    currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES * (bendDeflection / (float)8192));
};

//
//// Bridge driving functions
//

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
 */
void L298N::tick()
{
  /*
   If there is a period set for bridge 1, count the number of
   ticks that pass, and toggle the pin if the current period is reached.
   */
  if (currentPeriod[1]>0){
    currentTick[1]++;
    if (currentTick[1] >= currentPeriod[1]){
      step(1,2,3,4,5); // Bridge 1 is on pin 2,3,4,5
      currentTick[1]=0;
    }
  }
  if (currentPeriod[2]>0){
    currentTick[2]++;
    if (currentTick[2] >= currentPeriod[2]){
      step(2,6,7,8,9);
      currentTick[2]=0;
    }
  }
  if (currentPeriod[3]>0){
    currentTick[3]++;
    if (currentTick[3] >= currentPeriod[3]){
      step(3,10,11,12,13);
      currentTick[3]=0;
    }
  }
  if (currentPeriod[3]>0){
    currentTick[3]++;
    if (currentTick[3] >= currentPeriod[3]){
      step(4,14,15,16,17);
      currentTick[3]=0;
    }
  }
}


void L298N::step(byte bridgeNum, byte pin1, byte pin2, byte pin3, byte pin4) {
  //Switch directions if end has been reached
  if (currentPosition[bridgeNum] >= MAX_POSITION[bridgeNum]) {
    currentDir[bridgeNum] = 1;
  }
  else if (currentPosition[bridgeNum] <= 0) {
    currentDir[bridgeNum] = 0;
  }

  //Update currentPosition
  if (currentDir[bridgeNum] == 1){
    currentPosition[bridgeNum]--;
  }
  else {
    currentPosition[bridgeNum]++;
  }
	if (currentDir[bridgeNum] == 0) {
	if (currentStep[bridgeNum] >= 3) {
		currentStep[bridgeNum] = 0;
	}
	else {
		currentStep[bridgeNum]++;
	}
	}
	if (currentDir[bridgeNum] == 1) {
	if (currentStep[bridgeNum] <= 0) {
		currentStep[bridgeNum] = 3;
	}
	else {
		currentStep[bridgeNum]--;
	}
	}

  //Make a step in the right direction
switch (currentStep[bridgeNum]) { // Table of steps. Steps need to follow this order. If in reverse,
// we start from the bottom.
      case 0:  
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
        digitalWrite(pin3, HIGH);
        digitalWrite(pin4, LOW);
      break;
      case 1:   
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
        digitalWrite(pin3, HIGH);
        digitalWrite(pin4, LOW);

      break;
      case 2:  	 
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
        digitalWrite(pin3, LOW);
        digitalWrite(pin4, HIGH);
      break;
      case 3:  
 
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
        digitalWrite(pin3, LOW);
        digitalWrite(pin4, HIGH);
      break;
    }
}


//
//// UTILITY FUNCTIONS
//

//Not used now, but good for debugging...
void L298N::blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW);
}

// Immediately stops all drives
void L298N::haltAllDrives() {
  for (byte d=FIRST_BRIDGE;d<=LAST_BRIDGE;d++) {
    currentPeriod[d] = 0;
  }
}

//For a given bridge number, stop the note and set its position to zero
void L298N::reset(byte bridgeNum)
{
  currentPeriod[bridgeNum] = 0; // Stop note
  currentPosition[bridgeNum] = 0; // We're reset.
}

// Resets all the bridges simultaneously
void L298N::resetAll()
{

  // Stop all bridges and set to reverse
  for (byte d=FIRST_BRIDGE;d<=LAST_BRIDGE;d++) {
    currentPeriod[d] = 0;
  }

  // Reset all drives together
  for (unsigned int s=0;s<MAX_POSITION[0];s+=2){ //Half max because we're stepping directly (no toggle); grab max from index 0
    for (byte d=FIRST_BRIDGE;d<=LAST_BRIDGE;d++) {
    }
    delay(5);
  }

  // Return tracking to ready state
  for (byte d=FIRST_BRIDGE;d<=LAST_BRIDGE;d++) {
    currentPosition[d] = 0; // We're reset.
  }
}
} // namespace instruments
