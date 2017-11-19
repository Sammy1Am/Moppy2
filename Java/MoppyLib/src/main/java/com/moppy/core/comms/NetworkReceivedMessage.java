package com.moppy.core.comms;

/**
 * Because messages themselves don't carry any network-specific info, this subclass
 * adds that info for use locally.
 */
public class NetworkReceivedMessage extends MoppyMessage {
    
    private final String networkType; // Type of network (e.g. BridgeUDP or BridgeSerial)
    private final String networkIdentifier; // Unique identifier for this network (e.g. multicast IP:port or COM port name)
    private final String remoteIdentifier; // Identifier of remote device on the network if available (e.g. IP address)
    
    public NetworkReceivedMessage(byte[] messageBytes, String networkType, String networkIdentifier, String remoteIdentifier) {
        super(messageBytes);
        this.networkType = networkType;
        this.networkIdentifier = networkIdentifier;
        this.remoteIdentifier = remoteIdentifier;
    }

    public String getNetworkType() {
        return networkType;
    }

    public String getNetworkIdentifier() {
        return networkIdentifier;
    }

    public String getRemoteIdentifier() {
        return remoteIdentifier;
    }

}
