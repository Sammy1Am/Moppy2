/*
 * ShiftRegister.h
 * For bit shifters like the 74HC595N
 */

#ifndef SRC_MOPPYINSTRUMENTS_SHIFTREGISTER_H_
#define SRC_MOPPYINSTRUMENTS_SHIFTREGISTER_H_

#include <Arduino.h>
#include "MoppyTimer.h"
#include "MoppyInstrument.h"
#include "../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

namespace instruments {
  class ShiftRegister : public MoppyInstrument {
  public:
    void setup();

  protected:
    void sys_sequenceStop() override;
    void sys_reset() override;

    void dev_reset(uint8_t subAddress) override;
    void dev_noteOn(uint8_t subAddress, uint8_t payload[]) override;

  private:
    static uint8_t shiftData[];
    static uint8_t activeTicksLeft[];
    static boolean shouldShift;

    static void tick();
    static void blinkLED();
    static void startupSound(byte driveNum);
    static void shiftAllData();
    static void outputOn(byte outputNum);
    static void outputOff(byte outputNum);
    static void zeroOutputs();
  };
}

#endif /* SRC_MOPPYINSTRUMENTS_SHIFTREGISTER_H_ */
