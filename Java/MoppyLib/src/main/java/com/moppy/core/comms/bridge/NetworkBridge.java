/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.comms.bridge;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.NetworkReceivedMessage;
import java.io.Closeable;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;
import java.util.function.Consumer;

/**
 * Interface for a particular form of the Moppy network; used for sending
 * and receiving MoppyMessages.
 */
public abstract class NetworkBridge implements Closeable {
    
    private final Set<Consumer<NetworkReceivedMessage>> receivers = new HashSet<>();
    
    public abstract void connect() throws IOException;
    
    public abstract void sendMessage(MoppyMessage messageToSend) throws IOException;
    
    public abstract String getNetworkIdentifier();
    
    public void registerMessageReceiver(Consumer<NetworkReceivedMessage> messageConsumer) {
        receivers.add(messageConsumer);
    }
    
    public void deregisterMessageReceiver(Consumer<NetworkReceivedMessage> messageConsumer) {
        receivers.remove(messageConsumer);
    }
    
    /**
     * Called by the implementing class to pass a received message on to registered receivers. 
     */
    protected void messageToReceivers(NetworkReceivedMessage messageReceived) {
        receivers.forEach(c -> c.accept(messageReceived));
    }
}
