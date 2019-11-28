#include "MoppyTimer.h"

#ifdef ARDUINO_ARCH_AVR
#include <TimerOne.h>
#endif

void MoppyTimer::initialize(unsigned long microseconds, void (*isr)()) {
#ifdef ARDUINO_ARCH_AVR
    Timer1.initialize(microseconds);
    Timer1.attachInterrupt(isr);
#endif
}