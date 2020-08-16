/*
 * MoppyTimer.h
 * Attempt at making a high-precision timer that's relatively platform-agnostic
 */

#ifndef MOPPY_SRC_MOPPYINSTRUMENTS_MOPPYTIMER_H_
#define MOPPY_SRC_MOPPYINSTRUMENTS_MOPPYTIMER_H_

class MoppyTimer {
public:
    static void initialize(unsigned long microseconds, void (*isr)());
};

#endif /* MOPPY_SRC_MOPPYINSTRUMENTS_MOPPYTIMER_H_ */