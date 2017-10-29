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
        START_BYTE,           // Start byte
        SYSTEM_ADDRESS,           // System address
        0x01,           // Message body size
        CommandByte.SYS_PING});   // Ping command
    public static final MoppyMessage SYS_RESET = new MoppyMessage(new byte[]{START_BYTE, SYSTEM_ADDRESS, 0x01, CommandByte.SYS_RESET});
    public static final MoppyMessage SYS_START = new MoppyMessage(new byte[]{START_BYTE, SYSTEM_ADDRESS, 0x01, CommandByte.SYS_START});
    public static final MoppyMessage SYS_STOP = new MoppyMessage(new byte[]{START_BYTE, SYSTEM_ADDRESS, 0x01, CommandByte.SYS_STOP});
    

    protected MoppyMessage(byte[] messageBytes) {
        if (messageBytes.length < 4) {
            throw new IllegalArgumentException("Not enough bytes for a MoppyMessage!");
        }
        
        this.messageBytes = messageBytes;
    }
    
    public static class CommandByte {
        public static byte SYS_PING = (byte)0x80;
        public static byte SYS_PONG = (byte)0x81;
        public static byte SYS_RESET = (byte)0x82;
        public static byte SYS_START = (byte)0x83;
        public static byte SYS_STOP = (byte)0x84;
        
        public static byte DEV_RESET = 0x00;
        public static byte DEV_PLAYNOTE = 0x01;
        public static byte DEV_STOPNOTE = 0x02;
        public static byte DEV_BENDPITCH = 0x03;
    }
    
    public byte[] getMessageBytes(){
        return messageBytes;
    }
    
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
    
    public byte[] getMessageBody() {
        int bodyLength = isSystemMessage() ? messageBytes[2] : messageBytes[3];
        int bodyStart = isSystemMessage() ? 3 : 4;
        return Arrays.copyOfRange(messageBytes, bodyStart, bodyStart + bodyLength);
    }
    
    
}
