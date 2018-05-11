package com.moppy.core.comms;

import java.util.Arrays;

/**
 * Represents a message suitable for sending to a Moppy network.
 *
 * NOTE: Java treats ALL bytes as signed, but we're using them unsigned.  Casts / conversions will be required!
 */
public class MoppyMessage {
    private final byte[] messageBytes;

    public static final byte START_BYTE = (byte)0x4d;
    public static final byte SYSTEM_ADDRESS = (byte)0x00;

    public static final MoppyMessage SYS_PING = new MoppyMessage(new byte[]{
        START_BYTE,             // Start byte
        SYSTEM_ADDRESS,         // System address
        0x00,                   // Ignored device address
        0x01,                   // Message body size
        CommandByte.SYS_PING}); // Ping command
    public static final MoppyMessage SYS_RESET = new MoppyMessage(new byte[]{START_BYTE, SYSTEM_ADDRESS, 0x00, 0x01, CommandByte.SYS_RESET});
    public static final MoppyMessage SYS_START = new MoppyMessage(new byte[]{START_BYTE, SYSTEM_ADDRESS, 0x00, 0x01, CommandByte.SYS_START});
    public static final MoppyMessage SYS_STOP = new MoppyMessage(new byte[]{START_BYTE, SYSTEM_ADDRESS, 0x00, 0x01, CommandByte.SYS_STOP});


    protected MoppyMessage(byte[] messageBytes) {
        if (messageBytes.length < 5) {
            throw new IllegalArgumentException("Not enough bytes for a MoppyMessage!");
        }

        this.messageBytes = messageBytes;
    }

    public static class CommandByte {
        public static byte SYS_PING = (byte)0x80;
        public static byte SYS_PONG = (byte)0x81;
        public static byte SYS_RESET = (byte)0xff;
        public static byte SYS_START = (byte)0xfa;
        public static byte SYS_STOP = (byte)0xfc;

        public static byte DEV_RESET = 0x00;
        public static byte DEV_PLAYNOTE = 0x09;
        public static byte DEV_STOPNOTE = 0x08;
        public static byte DEV_BENDPITCH = 0x0e;
    }

    /**
     * Returns raw bytes that make up the message
     */
    public byte[] getMessageBytes(){
        return messageBytes;
    }

    /**
     * Returns target device address from the message
     */
    public byte getDeviceAddress() {
        return messageBytes[1];
    }

    public boolean isSystemMessage() {
        return getDeviceAddress() == SYSTEM_ADDRESS;
    }

    public byte getSubAddress() {
        if (isSystemMessage()) {
            throw new IllegalStateException("This is a system message and has no sub-address");
        } else {
            return messageBytes[2];
        }
    }

    /**
     * Returns the body of the message which includes the command byte and any additional bytes of command payload
     */
    public byte[] getMessageBody() {
        int bodyLength = messageBytes[3];
        return Arrays.copyOfRange(messageBytes, 4, 4 + bodyLength);
    }

    public byte getMessageCommandByte() {
        return getMessageBody()[0];
    }

    /**
     * Returns just the variable command payload at the end of the message (may be zero-length!)
     */
    public byte[] getMessageCommandPayload() {
        return Arrays.copyOfRange(getMessageBody(), 1, getMessageBody().length);
    }
}
