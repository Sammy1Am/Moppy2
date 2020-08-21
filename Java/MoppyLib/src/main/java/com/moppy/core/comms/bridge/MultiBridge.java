/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.comms.bridge;

import com.moppy.core.comms.MoppyMessage;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;

/**
 * A NetworkBridge implementation that supports multiple bridges.
 *
 * This allows, for example, multiple COM ports to be used, or a COM port to be used in conjunction
 * with a network bridge.
 */
public class MultiBridge extends NetworkBridge<Object> {

    private Set<NetworkBridge> bridges = new HashSet<>();

    /**
     * Adds a network bridge that will send/receive messages as part of the MultiBridge group.
     */
    public void addBridge(NetworkBridge bridgeToAdd) {
        // Any messages received by the underlying bridges should be sent to
        // the MultiBridge's receivers
        bridgeToAdd.registerMessageReceiver(this);
        bridges.add(bridgeToAdd);
    }

    public void removeBridge(NetworkBridge bridgeToAdd) {
        bridgeToAdd.deregisterMessageReceiver(this);
        bridges.remove(bridgeToAdd);
    }

    @Override
    public void connect() throws IOException {
        ArrayList<IOException> connectionExceptions = new ArrayList<>();
        bridges.forEach(b -> {
            try {
                b.connect();
            } catch (IOException ex) {
                connectionExceptions.add(ex);
                Logger.getLogger(MultiBridge.class.getName()).log(Level.WARNING, null, ex);
            }
        });

        if (connectionExceptions.size() > 0) {
            throw new IOException(String.format("%s exceptions thrown during network connection:\n%s",
                    connectionExceptions.size(),
                    connectionExceptions.stream().map(ex -> ex.getMessage()).collect(Collectors.joining("\n"))));
        }
    }
    
    @Override
    public void connect(Object connectionOption) throws IOException {
        connect();  // Ignore argument since we have no options
    }

    @Override
    public void sendMessage(MoppyMessage messageToSend) throws IOException {
        bridges.forEach(b -> {
            try {
                b.sendMessage(messageToSend);
            } catch (IOException ex) {
                // Potentially only one of the network interfaces here is throwing an exception, so
                // we don't want to rethrow-- instead we'll make sure it's logged and keep going
                Logger.getLogger(MultiBridge.class.getName()).log(Level.WARNING, null, ex);
            }
        });
    }

    @Override
    public void close() throws IOException {
        bridges.forEach(b -> {
            try {
                b.close();
            } catch (IOException ex) {
                // There's not much we can do if it fails to close (it's probably already closed).
                // Just log it and move on.
                Logger.getLogger(MultiBridge.class.getName()).log(Level.WARNING, null, ex);
            }
        });
    }

    @Override
    public String getNetworkIdentifier() {
        throw new UnsupportedOperationException("MultiBridge doesn't have a network ID");
    }

    @Override
    public boolean isConnected() {
        throw new UnsupportedOperationException("MultiBridge cannot report connected-ness");
    }

    @Override
    public List<Object> getConnectionOptions() {
        return Collections.EMPTY_LIST;
    }

    @Override
    public Object currentConnectionOption() {
        return null;
    }

}
