/*
 * MoppyNetwork.h
 *
 */

#ifndef SRC_MOPPYNETWORKS_MOPPYNETWORK_H_
#define SRC_MOPPYNETWORKS_MOPPYNETWORK_H_

// Definitions for command byte values

#define START_BYTE 0x4d
#define SYSTEM_ADDRESS 0x00

#define NETBYTE_SYS_PING 0x80
#define NETBYTE_SYS_PONG 0x81
#define NETBYTE_SYS_RESET 0xff
#define NETBYTE_SYS_START 0xfa
#define NETBYTE_SYS_STOP 0xfc

#define NETBYTE_DEV_RESET 0x00
#define NETBYTE_DEV_NOTEOFF 0x08
#define NETBYTE_DEV_NOTEON 0x09
#define NETBYTE_DEV_BENDPITCH 0x0e

// Non-core commands (listed here to prevent overlap)
#define NETBYTE_DEV_SETTARGETCOLOR 0x61
#define NETBYTE_DEV_SETBGCOLOR 0x62
#define NETBYTE_DEV_SETMOVEMENT 0x64

#endif /* SRC_MOPPYNETWORKS_MOPPYNETWORK_H_ */
