/*
 * L298N.h
 * Pinout info:
pin 2, IN1 for bridge 1
pin 3, IN2 for bridge 1
pin 4, IN3 for bridge 1
pin 5, IN4 for bridge 1
pin 6, IN1 for bridge 2
pin 7, IN2 for bridge 2
pin 8, IN3 for bridge 2
pin 9, IN4 for bridge 2
pin 10, IN1 for bridge 3
pin 11, IN2 for bridge 3
pin 12, IN3 for bridge 3
pin 13, IN4 for bridge 3
pin 14 (A0), IN1 for bridge 4
pin 15 (A1), IN2 for bridge 4
pin 16 (A2), IN3 for bridge 4
pin 17 (A3), IN4 for bridge 4
 */

#ifndef SRC_MOPPYINSTRUMENTS_L298N_H_
#define SRC_MOPPYINSTRUMENTS_L298N_H_

#include <Arduino.h>
#include "MoppyTimer.h"
#include "MoppyInstrument.h"
#include "../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

namespace instruments {
  class L298N : public MoppyInstrument {
  public:
    void setup();
  protected:
    void sys_sequenceStop() override;
    void sys_reset() override;

    void dev_reset(uint8_t subAddress) override;
    void dev_noteOn(uint8_t subAddress, uint8_t payload[]) override;
    void dev_noteOff(uint8_t subAddress, uint8_t payload[]) override;
    void dev_bendPitch(uint8_t subAddress, uint8_t payload[]) override;
  private:
    static int FIRST_BRIDGE;
    static int LAST_BRIDGE;
    static unsigned int MAX_POSITION[];
    static unsigned int currentPosition[];
    static int currentStep[];
    static int currentDir[];
    static unsigned int currentPeriod[];
    static unsigned int currentTick[];
    static unsigned int originalPeriod[];
    static void resetAll();
    static void step(byte bridgeNum, byte pin1, byte pin2, byte pin3, byte pin4);
    static void haltAllDrives();
    static void reset(byte bridgeNum);
    static void tick();
    static void blinkLED();
    static void startupSound(byte bridgeNum);
    static void L298Nvariables();
  };
}

#endif /* SRC_MOPPYINSTRUMENTS_L298N_H_ */
