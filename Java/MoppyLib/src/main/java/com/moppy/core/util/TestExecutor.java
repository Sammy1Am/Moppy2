/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.util;

import com.moppy.core.comms.MoppyMessageFactory;
import com.moppy.core.comms.bridge.BridgeUDP;
import java.io.IOException;
import java.net.UnknownHostException;

/**
 *
 * @author Sam
 */
public class TestExecutor {
    public static void main(String[] args) throws UnknownHostException, IOException, InterruptedException {
        try (BridgeUDP testBridge = new BridgeUDP()) {
            
            testBridge.connect();
            
            while (!Thread.interrupted()) {
                Thread.sleep(1000);
                testBridge.sendMessage(MoppyMessageFactory.devicePlayNote((byte)0x01, (byte)0x01, (byte)60));
                System.out.println("N");
                Thread.sleep(1000);
                testBridge.sendMessage(MoppyMessageFactory.devicePlayNote((byte)0x01, (byte)0x02, (byte)65));
                System.out.println("C");
                Thread.sleep(1000);
                testBridge.sendMessage(MoppyMessageFactory.devicePlayNote((byte)0x01, (byte)0x03, (byte)69));
                System.out.println("B");
                Thread.sleep(2000);
                testBridge.sendMessage(MoppyMessageFactory.deviceReset((byte)0x01));
            }
            
            
        }
    }
}
