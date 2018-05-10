package com.moppy.core.comms;

/**
 * Because messages themselves don't carry any network-specific info, this subclass
 * adds that info for use locally.
 */
public class NetworkReceivedMessage extends MoppyMessage {

    private final String networkType;
    private final String networkIdentifier;
    private final String remoteIdentifier;

    public NetworkReceivedMessage(byte[] messageBytes, String networkType, String networkIdentifier, String remoteIdentifier) {
        super(messageBytes);
        this.networkType = networkType;
        this.networkIdentifier = networkIdentifier;
        this.remoteIdentifier = remoteIdentifier;
    }

    /**
     * @return Type of network (e.g. BridgeUDP or BridgeSerial)
     */

    public String getNetworkType() {
        return networkType;
    }

    /**
     * @return Unique identifier for this network (e.g. multicast IP:port or COM port name)
     */
    public String getNetworkIdentifier() {
        return networkIdentifier;
    }

    /**
     * @return Identifier of remote device on the network if available (e.g. IP address)
     */
    public String getRemoteIdentifier() {
        return remoteIdentifier;
    }

}
