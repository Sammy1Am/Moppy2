package com.moppy.control;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.NetworkMessageConsumer;
import com.moppy.core.comms.NetworkReceivedMessage;
import com.moppy.core.comms.bridge.BridgeSerial;
import com.moppy.core.comms.bridge.BridgeUDP;
import com.moppy.core.comms.bridge.MultiBridge;
import com.moppy.core.comms.bridge.NetworkBridge;
import com.moppy.core.device.DeviceDescriptor;
import com.moppy.core.status.StatusBus;
import com.moppy.core.status.StatusUpdate;
import java.io.IOException;
import java.net.UnknownHostException;
import java.time.Duration;
import java.time.Instant;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;

/**
 * A class for managing connecting, disconnecting, and status for Control's network connection(s)
 */
public class NetworkManager implements NetworkMessageConsumer {

    /**
     * StatusBus for updating local components about network changes.
     */
    private final StatusBus statusBus;

    private final MultiBridge multiBridge = new MultiBridge();
    private final HashMap<String, NetworkBridge> networkBridges = new HashMap<>();
    private final ConcurrentHashMap<DeviceDescriptor, Instant> recentlySeenDevices = new ConcurrentHashMap<>();

    private Thread pingerThread;

    public NetworkManager(StatusBus statusBus) {
        this.statusBus = statusBus;
        multiBridge.registerMessageReceiver(this); // Register to receive network messages to look for pongs

        try {
            BridgeUDP udpBridge = new BridgeUDP();
            networkBridges.put(udpBridge.getNetworkIdentifier(), udpBridge);
        } catch (UnknownHostException ex) {
            Logger.getLogger(NetworkManager.class.getName()).log(Level.SEVERE, null, ex);
        }

        BridgeSerial.getAvailableSerials()
                .forEach(serial -> {
                    BridgeSerial serialBridge = new BridgeSerial(serial);
                    networkBridges.put(serialBridge.getNetworkIdentifier(), serialBridge);
                });

    }

    public void start() {
        // Create and start listener thread
        NetworkPinger listener = new NetworkPinger(multiBridge, recentlySeenDevices, statusBus);
        pingerThread = new Thread(listener);
        pingerThread.start();
    }

    //TODO: Should probably auto-close or somehow have a way to stop the pinger thread

    /**
     * For the purposes of being able to send or register to receive messages, returns
     * the primary NetworkBridge being managed.
     */
    public NetworkBridge getPrimaryBridge() {
        return multiBridge;
    }

    // The NetworkManager is primarily concerned with receiving pong messages from the network
    // so that it can update its current list of devices.
    @Override
    public void acceptNetworkMessage(NetworkReceivedMessage networkMessage) {

        // Whenever we receive a pong from one of the networks, update the device in
        // recentlySeenDevices with the time it was last seen
        if (networkMessage.isSystemMessage() && networkMessage.getMessageCommandByte() == MoppyMessage.CommandByte.SYS_PONG) {
            DeviceDescriptor dd = DeviceDescriptor.builder()
                    .networkAddress(String.format("%s - %s",networkMessage.getNetworkIdentifier(),networkMessage.getRemoteIdentifier()))
                    .deviceAddress(networkMessage.getMessageCommandPayload()[0])
                    .minSubAddress(networkMessage.getMessageCommandPayload()[1])
                    .maxSubAddress(networkMessage.getMessageCommandPayload()[2])
                    .build();

            Instant lastSeen = recentlySeenDevices.put(dd, Instant.now());
            // If this device had never been seen before, we have a new device!  Let everyone know!
            if (lastSeen == null) {
                statusBus.receiveUpdate(StatusUpdate.NET_DEVICES_CHANGED);
            }
        }
    }

    /**
     * Returns a Map of unique network bridge identifiers and the network bridge
     * @return
     */
    public Map<String, NetworkBridge> getAvailableNetworkBridges() {
        return networkBridges.entrySet().stream()
                .collect(Collectors.toMap(Entry<String, NetworkBridge>::getKey, entry -> entry.getValue()));
    }

    /**
     * Returns a Set of DeviceDescriptors for devices for who we recently received a pong.
     * @return
     */
    public Set<DeviceDescriptor> getRecentlySeenDevices() {
        return recentlySeenDevices.keySet();
    }

    public void connectBridge(String bridgeIdentifier, Object connectionOption) throws IOException {
        try {
            networkBridges.get(bridgeIdentifier).connect(connectionOption);
            networkBridges.get(bridgeIdentifier).registerMessageReceiver(multiBridge);
            multiBridge.addBridge(networkBridges.get(bridgeIdentifier));
        } finally {
            statusBus.receiveUpdate(StatusUpdate.NET_STATUS_CHANGED);
        }
    }

    public void closeBridge(String bridgeIdentifier) throws IOException {
        try {
            multiBridge.removeBridge(networkBridges.get(bridgeIdentifier));
            networkBridges.get(bridgeIdentifier).deregisterMessageReceiver(multiBridge);
            networkBridges.get(bridgeIdentifier).close();
        } finally {
            statusBus.receiveUpdate(StatusUpdate.NET_STATUS_CHANGED);
        }
    }

    /**
     * Occasionally pings the network looking for new devices.  Also culls
     * devices not seen for a while from the list.
     */
    private static class NetworkPinger implements Runnable {

        private final NetworkBridge bridgeToPing;
        private final ConcurrentHashMap<DeviceDescriptor, Instant> recentlySeenDevices;
        private final StatusBus statusBus; // Status bus for alerting to device removals

        public NetworkPinger(NetworkBridge bridgeToPing, ConcurrentHashMap<DeviceDescriptor, Instant> recentlySeenDevices, StatusBus statusBus) {
            this.bridgeToPing = bridgeToPing;
            this.recentlySeenDevices = recentlySeenDevices;
            this.statusBus = statusBus;
        }

        @Override
        public void run() {

            while (!Thread.interrupted()) {
                // Send a ping
                try {
                    bridgeToPing.sendMessage(MoppyMessage.SYS_PING);
                } catch (IOException ex) {
                    // If for some reason we can't send the message, just log and carry on (hopefully whatever's wrong
                    // will resolve itself again, but we don't want to kill the pinger)
                    Logger.getLogger(NetworkManager.class.getName()).log(Level.WARNING, null, ex);
                }

                // Wait a bit for responses and because we don't need to ping constantly
                try {
                    Thread.sleep(3000);
                } catch (InterruptedException ex) {
                    break;
                }

                // Cull devices not seen for 7 seconds
                boolean devicesCulled = recentlySeenDevices.values().removeIf(lastSeen -> Duration.ofSeconds(7)
                        .minus(Duration.between(lastSeen, Instant.now()))
                        .isNegative());
                if (devicesCulled) {
                    statusBus.receiveUpdate(StatusUpdate.NET_DEVICES_CHANGED);
                }
            }
        }

    }
}
