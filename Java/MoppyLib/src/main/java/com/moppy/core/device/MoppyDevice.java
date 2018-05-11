/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.device;

import com.moppy.core.comms.MoppyMessage;

/**
 * Convenience class to automatically parse MoppyMessages into Java method calls
 */
public abstract class MoppyDevice {

    /**
     * Returns true if this device is listening for the specified address.
     *
     * Must be handled by implementing class
     * @param deviceAddress
     * @param subAddress
     * @return
     */
    public abstract boolean matchesAddress(byte deviceAddress, byte subAddress);

    public void handleMessage(MoppyMessage incommingMessage) {
        if (incommingMessage.isSystemMessage()) {
            switch (incommingMessage.getMessageCommandByte()) {
                case (byte)0x80:
                    gotSystemPing();
                    break;
                case (byte)0x82:
                    systemReset();
                    break;
            }
        } else if (matchesAddress(incommingMessage.getDeviceAddress(), incommingMessage.getSubAddress())){
            switch (incommingMessage.getMessageCommandByte()) {
                case 0x00:
                    deviceReset(incommingMessage.getDeviceAddress(), incommingMessage.getSubAddress());
                    break;
                case 0x09:
                    devicePlayNote(incommingMessage.getDeviceAddress(),
                            incommingMessage.getSubAddress(),
                            incommingMessage.getMessageCommandPayload()[0]);
                    break;
                case 0x08:
                    deviceStopNote(incommingMessage.getDeviceAddress(),
                            incommingMessage.getSubAddress(),
                            incommingMessage.getMessageCommandPayload()[0]);
                    break;
                case 0x0e:
                    deviceBendPitch(incommingMessage.getDeviceAddress(), incommingMessage.getSubAddress(),
                            (short)((incommingMessage.getMessageCommandPayload()[0] << 8) | incommingMessage.getMessageCommandPayload()[1]));
                    break;
            }
        }
        // Else this message wasn't for us
    }

    ////
    // System Message Handlers
    ////

    /**
     * Override to respond to system pings
     */
    public void gotSystemPing() {

    }

    /**
     * Override to provide system reset functionality
     */
    public void systemReset() {

    }

    ////
    // Device Message Handlers
    ////

    /**
     * Override to provide reset functionality
     */
    public void deviceReset(byte deviceAddress, byte subAddress) {
    }

    /**
     * Override to provide play note functionality
     */
    public void devicePlayNote(byte deviceAddress, byte subAddress, byte noteNumber) {
    }

    /**
     * Override to provide stop note functionality
     */
    public void deviceStopNote(byte deviceAddress, byte subAddress, byte noteNumber) {
    }

    /**
     * Override to provide pitch bending functionality
     */
    public void deviceBendPitch(byte deviceAddress, byte subAddress, short bendAmount) {
    }


}
