/*
 * ShiftedFloppyDrives.h
 * Floppy drives connected to shift register(s)
 */

#ifndef SRC_MOPPYINSTRUMENTS_FLOPPYDRIVES_H_
#define SRC_MOPPYINSTRUMENTS_FLOPPYDRIVES_H_

#include "../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"
#include "MoppyInstrument.h"
#include "MoppyTimer.h"
#include <Arduino.h>
#include <SPI.h>
namespace instruments {
  class ShiftedFloppyDrives : public MoppyInstrument {
  public:
    void setup();

    static const int DATA_PIN = 11;  //SER
    static const int CLOCK_PIN = 13; //SRCLK
    static const int LATCH_PIN = 4; //RCLK

  protected:
      void sys_sequenceStop() override;
      void sys_reset() override;

      void dev_reset(uint8_t subAddress) override;
      void dev_noteOn(uint8_t subAddress, uint8_t payload[]) override;
      void dev_noteOff(uint8_t subAddress, uint8_t payload[]) override;
      void dev_bendPitch(uint8_t subAddress, uint8_t payload[]) override;

  private:
      // First drive being used for floppies, and the last drive.  Used for calculating
      // step and direction pins.
      static const byte FIRST_DRIVE = 1;
      static const byte LAST_DRIVE = 8; // This may also determine the size of some arrays

      // Maximum note number to attempt to play on floppy drives.  It's possible higher notes may work,
      // but they may also cause instability.
      static const byte MAX_FLOPPY_NOTE = 71;

      
      static unsigned int MAX_POSITION[LAST_DRIVE];
      static unsigned int currentPosition[LAST_DRIVE];
      static uint8_t currentState[2];
      static unsigned int currentPeriod[];
      static unsigned int currentTick[];
      static unsigned int originalPeriod[];

      static void tick();
      static void resetAll();
      static void togglePin(byte driveNum, byte pin, byte direction_pin);
      static void shiftBits();
      static void haltAllDrives();
      static void reset(byte driveNum);
      static void blinkLED();
      static void startupSound(byte driveNum);
  };
}



#endif /* SRC_MOPPYINSTRUMENTS_FLOPPYDRIVES_H_ */
