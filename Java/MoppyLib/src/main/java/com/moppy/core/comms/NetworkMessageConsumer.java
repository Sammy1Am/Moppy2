package com.moppy.core.comms;

/**
 * Interface for any classes that want to receive messages from the Moppy Network.
 */
public interface NetworkMessageConsumer {
    public void acceptNetworkMessage(NetworkReceivedMessage networkMessage);
}
