/*
 * EasyDrivers.cpp
 * @author : Sammy1Am, modified for the EasyDriver/A3967 by tobiasfrck
 * Output for controlling EasyDrivers.
 */
#include "MoppyInstrument.h"
#include "EasyDrivers.h"

namespace instruments {
// This is used for calculating step and direction pins.
const byte FIRST_DRIVER = 1;
const byte LAST_DRIVER = 3;  // This sketch can handle only up to 3 drivers (the max for Arduino Uno)


// Maximum note number to attempt to play on easydrivers.  It's possible higher notes may work,
// but they need to be added in "MoppyInstrument.h".
const byte MAX_DRIVER_NOTE = 119;


/*NOTE: The arrays below contain unused zero-indexes to avoid having to do extra
 * math to shift the 1-based subAddresses to 0-based indexes here.  Unlike the previous
 * version of Moppy, we *will* be doing math to calculate which driver maps to which pin,
 * so there are as many values as drivers (plus the extra zero-index)
 */


// Microstep Resolution of each stepper motor:
//                              {MS1,MS2,MS1,MS2,MS1,MS2}
unsigned int stepResolution[] = {LOW,LOW,LOW,LOW,LOW,LOW};
/*
+------+-------+-------------------------+
| MS1  |  MS2  |   Microstep Resolution  |
+------+-------+-------------------------+
| L    | L     | Full Step (2 Phase)     |
+------+-------+-------------------------+
| H    | L     | Half Step               |
+------+-------+-------------------------+
| L    | H     | Quarter Step            |
+------+-------+-------------------------+
| H    | H     | Eigth Step              |
+------+-------+-------------------------+
 */

/*
NOTE: This integer controls the "resetAll" function, and should contain the highest value maximum poisitions of all EasyDrivers
 */
// Uncomment this if you want to be able to reset the drivers!
//unsigned int max_position = 7200;


/*Array to keep track of state of each pin.  Even indexes track the step-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse <- This depends on the wiring of the stepper motor with the EasyDriver.
 */
int EasyDrivers::currentState[] = {0,0,LOW,LOW,LOW,LOW,LOW,LOW};

// Current period assigned to each driver.  0 = off.  Each period is two-ticks (as defined by
// TIMER_RESOLUTION in MoppyInstrument.h) long.
unsigned int EasyDrivers::currentPeriod[] = {0,0,0,0,0};

// Tracks the current tick-count for each driver (see EasyDrivers::tick() below)
unsigned int EasyDrivers::currentTick[] = {0,0,0,0,0};

// The period originally set by incoming messages (prior to any modifications from pitch-bending)
unsigned int EasyDrivers::originalPeriod[] = {0,0,0,0,0};

void EasyDrivers::setup() {

  // Prepare pins (0 and 1 are reserved for Serial communications)
  pinMode(2, OUTPUT); // Step pin 1
  pinMode(3, OUTPUT); // Direction pin 1
  pinMode(4, OUTPUT); // MS1 pin 1
  pinMode(5, OUTPUT); // MS2 pin 1
  pinMode(14, INPUT_PULLUP); // Front Direction-Switch 1
  pinMode(15, INPUT_PULLUP); // Rear Direction-Switch 1

  pinMode(6, OUTPUT); // Step pin 2
  pinMode(7, OUTPUT); // Direction pin 2
  pinMode(8, OUTPUT); // MS1 pin 2
  pinMode(9, OUTPUT); // MS2 pin 2
  pinMode(16, INPUT_PULLUP); // Front Direction-Switch 2
  pinMode(17, INPUT_PULLUP); // Rear Direction-Switch 2

  pinMode(10, OUTPUT); // Step pin 3
  pinMode(11, OUTPUT); // Direction pin 3
  pinMode(12, OUTPUT); // MS1 pin 3
  pinMode(13, OUTPUT); // MS2 pin 3
  pinMode(18, INPUT_PULLUP); // Front Direction-Switch 3
  pinMode(19, INPUT_PULLUP); // Rear Direction-Switch 3


  // Set the step resolution of each driver
  for(byte respin = 0;respin<LAST_DRIVER;respin++) {
    digitalWrite(respin*4+4,stepResolution[respin*2]);
    digitalWrite(respin*4+5,stepResolution[respin*2+1]);
  }

  // With all pins setup, let's do a first run reset
  resetAll();

  delay(500); // Wait a half second for safety

  // Setup timer to handle interrupts for drivers driving
  MoppyTimer::initialize(TIMER_RESOLUTION, tick);

  // If MoppyConfig wants a startup sound, play the startupSound on the
  // first driver.
  if (PLAY_STARTUP_SOUND) {
    startupSound(FIRST_DRIVER);
    delay(500);
    resetAll();
  }
}

// Play startup sound to confirm driver functionality
void EasyDrivers::startupSound(byte driverNum) {
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
      currentPeriod[driverNum] = chargeNotes[i++];
    }
  }
}

//
//// Message Handlers
//

void EasyDrivers::sys_reset() {
    resetAll();
}

void EasyDrivers::sys_sequenceStop() {
    haltAllDrivers();
}

void EasyDrivers::dev_reset(uint8_t subAddress) {
    if (subAddress == 0x00) {
        resetAll();
    } else {
        reset(subAddress);
    }
}

void EasyDrivers::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    // Set the current period to the new value to play it immediately
    // Also set the originalPeriod in-case we pitch-bend
    if (payload[0] <= MAX_DRIVER_NOTE) {
        currentPeriod[subAddress] = originalPeriod[subAddress] = noteDoubleTicks[payload[0]];
    }
}

void EasyDrivers::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    currentPeriod[subAddress] = originalPeriod[subAddress] = 0;
}

void EasyDrivers::dev_bendPitch(uint8_t subAddress, uint8_t payload[]) {
    // A value from -8192 to 8191 representing the pitch deflection
    int16_t bendDeflection = payload[0] << 8 | payload[1];

    // A whole octave of bend would double the frequency (halve the the period) of notes
    // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
    //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
    currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES * (bendDeflection / (float)8192));
}

//
//// Driver driving functions
//

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
 */
void EasyDrivers::tick()
{
  /*
   If there is a period set for step pin 2, count the number of
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
      togglePin(2,6,7);
      currentTick[2]=0;
    }
  }
  if (currentPeriod[3]>0){
    currentTick[3]++;
    if (currentTick[3] >= currentPeriod[3]){
      togglePin(3,10,11);
      currentTick[3]=0;
    }
  }
}

void EasyDrivers::togglePin(byte driverNum, byte pin, byte direction_pin) {
// Switch directions if either end has been reached.
  if (digitalRead(driverNum*2+12)==LOW) { // If front direction pin is on, change direction.
    currentState[direction_pin] = HIGH;
    digitalWrite(direction_pin,HIGH);
  }
  else if (digitalRead(driverNum*2+13)==LOW) { // If rear direction pin is on, change direction.
    currentState[direction_pin] = LOW;
    digitalWrite(direction_pin,LOW);
  }

  // Pulse the step pin
  digitalWrite(pin,currentState[pin]);
  currentState[pin] = ~currentState[pin];
}


//
//// UTILITY FUNCTIONS
//

// Not used now, but good for debugging...
void EasyDrivers::blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW);
}

// Immediately stops all drivers
void EasyDrivers::haltAllDrivers() {
  for (byte d=FIRST_DRIVER;d<=LAST_DRIVER;d++) {
    currentPeriod[d] = 0;
  }
}

// For a given driver number, runs e.g. the scanner-head all the way back to the rear
void EasyDrivers::reset(byte driverNum)
{
  currentPeriod[driverNum] = 0; // Stop note

  byte stepPin = (driverNum - 1) * 4 + 2; //2, 6, 10

  // Uncomment this if you want to be able to reset the drivers!
  /*
  digitalWrite(stepPin+1,HIGH); // Go in reverse
  while(digitalRead(driverNum*2+13)==HIGH) { //While the rear direction-switch is not triggered move backwards.
    digitalWrite(stepPin,HIGH);
    digitalWrite(stepPin,LOW);
  }
  */
  digitalWrite(stepPin,LOW);
  currentState[stepPin] = LOW;
  digitalWrite(stepPin+1,LOW);
  currentState[stepPin+1] = LOW; // Ready to go forward.
}

// Resets all the drivers simultaneously
void EasyDrivers::resetAll()
{

  // Stop all drivers and set to reverse
  for (byte d=FIRST_DRIVER;d<=LAST_DRIVER;d++) {
    byte stepPin = (d - 1) * 4 + 2; //2, 6, 10
    currentPeriod[d] = 0;
    digitalWrite(stepPin+1,HIGH);
  }

  // Reset all drivers together
  // Uncomment this if you want to be able to reset the drivers!
  /*
  for (unsigned int s=0;s<max_position;s++){ // This is kept because it provides the convenience, that all drivers reset at the same time.
    for (byte d=FIRST_DRIVER;d<=LAST_DRIVER;d++) {
      byte stepPin = (d - 1) * 4 + 2; //2, 6, 10
      if(digitalRead(d*2+13)==HIGH) { //If the rear direction-switch is not triggered move backwards.
        digitalWrite(stepPin,HIGH);
        digitalWrite(stepPin,LOW);
      } else {
        digitalWrite(stepPin,LOW);
      }
    }
    delay(5);
  }
*/

  // Return tracking to ready state
  for (byte d=FIRST_DRIVER;d<=LAST_DRIVER;d++) {
    byte stepPin = (d - 1) * 4 + 2; //2, 6, 10
    currentState[stepPin] = LOW;
    digitalWrite(stepPin+1,LOW);
    currentState[stepPin+1] = LOW; // Ready to go forward.
  }
}
} // namespace instruments
