package com.moppy.core.comms;

import static com.moppy.core.comms.MoppyMessage.CommandByte.*;
import static com.moppy.core.comms.MoppyMessage.START_BYTE;
import static com.moppy.core.comms.MoppyMessage.SYSTEM_ADDRESS;

/**
 * Class for building common MoppyMessages.
 */
public class MoppyMessageFactory {
    // System messages
    public static MoppyMessage systemPong(byte deviceAddress, byte minSubAddress, byte maxSubAddress) {
        return new MoppyMessage(new byte[]{START_BYTE, SYSTEM_ADDRESS, 0x00, 0x04, SYS_PONG, deviceAddress, minSubAddress, maxSubAddress});
    }

    // Device messages
    public static MoppyMessage deviceReset(byte deviceAddress) {
        return new MoppyMessage(new byte[]{START_BYTE, deviceAddress, 0x00, 0x01, DEV_RESET});
    }

    public static MoppyMessage devicePlayNote(byte deviceAddress, byte subAddress, byte noteNumber, byte noteVelocity) {
        return new MoppyMessage(new byte[]{START_BYTE, deviceAddress, subAddress, 0x03, DEV_PLAYNOTE, noteNumber, noteVelocity});
    }

    public static MoppyMessage deviceStopNote(byte deviceAddress, byte subAddress, byte noteNumber) {
        return new MoppyMessage(new byte[]{START_BYTE, deviceAddress, subAddress, 0x02, DEV_STOPNOTE, noteNumber});
    }

    public static MoppyMessage devicePitchBend(byte deviceAddress, byte subAddress, short bendAmount) {
        return new MoppyMessage(new byte[]{START_BYTE, deviceAddress, subAddress, 0x03, DEV_BENDPITCH,
            (byte)((bendAmount >> 8) & 0xff), (byte)(bendAmount & 0xff)});
    }

    /**
     * This method is not a recommended way to create well-structured MoppyMessages, but is available
     * primarily for NetworkBridges to take advantage of.
     *
     * No validation is performed by this method!
     *
     * @param bytes Raw byte array for MoppyMessage
     * @return A MoppyMessage containing the provided bytes.
     */
    public static MoppyMessage fromBytes(byte[] bytes) {
        return new MoppyMessage(bytes);
    }

    public static NetworkReceivedMessage networkReceivedFromBytes(byte[] bytes, String networkType, String networkIdentifier, String remoteIdentifier) {
        return new NetworkReceivedMessage(bytes, networkType, networkIdentifier, remoteIdentifier);
    }
}
