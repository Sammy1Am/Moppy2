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
import com.moppy.core.comms.NetworkMessageConsumer;
import java.util.List;

/**
 * Interface for a particular form of the Moppy network; used for sending
 * and receiving MoppyMessages.
 */
public abstract class NetworkBridge<CONNOPTION> implements Closeable, NetworkMessageConsumer {

    private final Set<NetworkMessageConsumer> receivers = new HashSet<>();

    public abstract void connect(CONNOPTION connectionOption) throws IOException;
    
    public abstract void connect() throws IOException;

    public abstract boolean isConnected();

    public abstract void sendMessage(MoppyMessage messageToSend) throws IOException;

    public abstract String getNetworkIdentifier();
    
    public abstract List<CONNOPTION> getConnectionOptions();
    
    public abstract CONNOPTION currentConnectionOption();

    public void registerMessageReceiver(NetworkMessageConsumer messageConsumer) {
        receivers.add(messageConsumer);
    }

    public void deregisterMessageReceiver(NetworkMessageConsumer messageConsumer) {
        receivers.remove(messageConsumer);
    }

    /**
     * Called by network leaves to pass messages from the network.  Because
     * any class extending NetworkBridge automatically inherits this method as well as the
     * NetworkMessageConsumer interface, messages will pass through bridge classes to the first
     * non-NetworkBridge NetworkMessageConsumer.
     */
    @Override
    public void acceptNetworkMessage(NetworkReceivedMessage messageReceived) {
        receivers.forEach(c -> c.acceptNetworkMessage(messageReceived));
    }
}
