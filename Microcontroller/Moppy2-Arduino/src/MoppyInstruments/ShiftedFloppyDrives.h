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
    static const int LATCH_PIN = 2; //RCLK

protected:
    void sys_sequenceStop() override;
    void sys_reset() override;

    void dev_reset(uint8_t subAddress) override;
    void dev_noteOn(uint8_t subAddress, uint8_t payload[]) override;
    void dev_noteOff(uint8_t subAddress, uint8_t payload[]) override;
    void dev_bendPitch(uint8_t subAddress, uint8_t payload[]) override;
    void deviceMessage(uint8_t subAddress, uint8_t command, uint8_t payload[]);

private:
    static const byte LAST_DRIVE = 8; // Number of drives being used.  This determines the size of some arrays.

    // Maximum note number to attempt to play on floppy drives.  It's possible higher notes may work,
    // but they may also cause instability.
    static const byte MAX_FLOPPY_NOTE = 71;

    static unsigned int MAX_POSITION[LAST_DRIVE];
    static unsigned int MIN_POSITION[LAST_DRIVE];
    static unsigned int currentPosition[LAST_DRIVE];
    static uint8_t stepBits;      // Bits that represent the current state of the step pins
    static uint8_t directionBits; // Bits that represent the current state of the direction pins
    static unsigned int currentPeriod[LAST_DRIVE];
    static unsigned int currentTick[LAST_DRIVE];
    static unsigned int originalPeriod[LAST_DRIVE];

    static void tick();
    static void resetAll();
    static void togglePin(byte driveIndex);
    static void shiftBits();
    static void haltAllDrives();
    static void reset(byte driveNum);
    static void blinkLED();
    static void startupSound(byte driveIndex);
    static void setMovement(byte driveIndex, bool movementEnabled);
};
} // namespace instruments

#endif /* SRC_MOPPYINSTRUMENTS_FLOPPYDRIVES_H_ */