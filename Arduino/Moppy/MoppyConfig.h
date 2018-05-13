/*
 * MoppyConfig.h
 */

#ifndef SRC_MOPPYCONFIG_H_
#define SRC_MOPPYCONFIG_H_

////
// Moppy Device configuration
////

// Play a startup sound when the Arduino is initialized.  Helpful
// for debugging purposes, but can be turned off once everything
// is working
#define PLAY_STARTUP_SOUND true

// Device address for this Arduino (only messages sent to this address
// will be processed.
#define DEVICE_ADDRESS 0x01

// Minimum and maximum sub-addresses that messages will be processed for.
// E.g. if you have 8 drives this would be 1 and 8.  If you have 16, 1 and 16.
#define MIN_SUB_ADDRESS 1
#define MAX_SUB_ADDRESS 8


#endif /* SRC_MOPPYCONFIG_H_ */
