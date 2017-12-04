#include "MoppySerial.h"
#include <TimerOne.h>

#define DATA_PIN 2
#define SHIFT_CLOCK_PIN 3
#define LATCH_PIN 4
#define MASTER_CLEAR_PIN 5
#define OUTPUT_ENABLE_PIN 6

// Uncomment the appropriate networking class for your setup
MoppySerial network = MoppySerial(&sysMessage, &devMessage);
// MoppyUDP network = TODO...

// First and last supported notes (any notes outside this range will be ignored
#define NUM_NOTES 24
const uint8_t FIRST_NOTE = 48;
const uint8_t LAST_NOTE = FIRST_NOTE + NUM_NOTES;

#define RESOLUTION 1000 //Microsecond resolution starting and ending notes
#define PULSE_TICKS 50 // Length of "on" pulse for solenoids in ticks (e.g. RESOLUTION * PULSE_TICKS)

#define SHIFT_DATA_MAX_INDEX 2
#define SHIFT_DATA_BYTES 3
uint8_t shiftData[SHIFT_DATA_BYTES]; // 8*3 = 24
uint8_t activeTicksLeft[NUM_NOTES];

boolean shouldShift = false; // When true, a shift will occur during the next tick

void setup() {
  
  // Prepare pins (0 and 1 are reserved for Serial communications)
  pinMode(DATA_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  pinMode(13, OUTPUT); // Build in LED for blinking

  // With all pins setup, let's do a first run reset
  zeroOutputs();
  delay(1000); // Wait a second for safety

  // Setup timer to handle interrupts for floppy driving
  Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function

  // Start receiving messages
  network.begin();
}

void loop() {
  network.readMessages();

  if (shouldShift) {
    shouldShift = false;
    shiftAllData();
  }
}

//
//// Message Handlers
//

void sysMessage(uint8_t command, uint8_t payload[]) {
  switch(command) {
    case 0x80: // Ping request
      digitalWrite(13, !digitalRead(13)); // Toggle LED
      network.sendPong();
      break;
    case 0x82: // System reset
      zeroOutputs();
      break;
    case 0x83: // Sequence start
      // Nothing to do here yet
      break;
    case 0x84: // Sequence stop
      zeroOutputs();
      break;
  }
}

void devMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]) {
  switch(command) {
    case 0x00: // Reset
      zeroOutputs(); // SubAddress unimportant here since we only have one
      break;
    case 0x01: // Note On
      if (payload[0]>=FIRST_NOTE && payload[0]<=LAST_NOTE) {
        outputOn(payload[0]-FIRST_NOTE);
        activeTicksLeft[payload[0]-FIRST_NOTE] = PULSE_TICKS;
        shouldShift = true;
      }
      
      break;
    case 0x02: // Note Off
      // Ignore these, the solenoids are pulsed the same length for all notes
      break;
    case 0x03: //Pitch bend
      // Ignore these, we can't bend pitch
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

void tick()
{
  for (byte n=0;n<NUM_NOTES;n++) {
    if (activeTicksLeft[n] > 0) {
      if (--activeTicksLeft[n] == 0) {
        outputOff(n);
        shouldShift=true;
      }
    }
  }
}

//
//// UTILITY FUNCTIONS
//

//Not used now, but good for debugging...
void blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW); 
}

void shiftAllData()
{
  digitalWrite(LATCH_PIN, LOW);
  for (int i=SHIFT_DATA_BYTES-1;i>=0;i--){
    shiftOut(DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, shiftData[i]);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

void outputOn(byte outputNum){
  bitSet(shiftData[outputNum/8],outputNum%8);
}

void outputOff(byte outputNum){
  bitClear(shiftData[outputNum/8],outputNum%8);
}

void zeroOutputs() {
  for (byte i=0;i<SHIFT_DATA_BYTES;i++){
    shiftData[i] = 0;
  }
  shiftAllData();
}

