/*
 * FloppyDrives.cpp
 *
 * Output for controlling floppy drives.  The _original_ Moppy instrument!
 */
#include "../MoppyInstruments/FloppyDrives.h"

// First drive being used for floppies, and the last drive.  Used for calculating
// step and direction pins.
const byte FIRST_DRIVE = 1;
const byte LAST_DRIVE = 8;  // This sketch can handle only up to 9 drives (the max for Arduino Uno)

#define RESOLUTION 40 //Microsecond resolution for notes

// TODO: These periods and noteTicks should probably be moved out into their own library

// notePeriods stores the period of notes in microseconds, this will be converted to
// ticks by dividing by RESOLUTION
unsigned int notePeriods[128] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        30578, 28861, 27242, 25713, 24270, 22909, 21622, 20409, 19263, 18182, 17161, 16198, //C1 - B1
        15289, 14436, 13621, 12856, 12135, 11454, 10811, 10205, 9632, 9091, 8581, 8099, //C2 - B2
        7645, 7218, 6811, 6428, 6068, 5727, 5406, 5103, 4816, 4546, 4291, 4050, //C3 - B3
        3823, 3609, 3406, 3214, 3034, 2864, 2703, 2552, 2408, 2273, 2146, 2025, //C4 - B4
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

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
unsigned int MAX_POSITION[] = {158,158,158,158,158,158,158,158,158,158};

//Array to track the current position of each floppy head.
unsigned int currentPosition[] = {0,0,0,0,0,0,0,0,0,0};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse
 */
int currentState[] = {0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

//Current period assigned to each drive.  0 = off.  Each period is of the length specified by the RESOLUTION
//variable above.  i.e. A period of 10 is (RESOLUTION x 10) microseconds long.
unsigned int currentPeriod[] = {0,0,0,0,0,0,0,0,0,0};

//Current tick
unsigned int currentTick[] = {0,0,0,0,0,0,0,0,0,0};

//unsigned int microsToTicks(unsigned int m) { return m/(RESOLUTION*2); } // RESOLUTION * 2 because we need two ticks for one step

void FloppyDrives::setup() {
  // Create a noteTicks array that contains the resolved tick-ammount based on resolution and
  // microsecond periods
  //transform(MICRO_PERIODS, MICRO_PERIODS+128, noteTicks, microsToTicks);
  for (int i=0;i<128;i++){
    notePeriods[i] = notePeriods[i]/(RESOLUTION*2); // RESOLUTION * 2 because we need two ticks for one step
  }

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
  delay(1000); // Wait a second for safety

  // Setup timer to handle interrupts for floppy driving
  Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function


}

//
//// Message Handlers
//

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
      currentPeriod[subAddress] = notePeriods[payload[0]];
      break;
    case NETBYTE_DEV_NOTEOFF: // Note Off
      currentPeriod[subAddress] = 0;
      break;
    case NETBYTE_DEV_BENDPITCH: //Pitch bend
      // TODO
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
