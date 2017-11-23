/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.comms.bridge;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.MoppyMessageFactory;
import com.moppy.core.comms.NetworkReceivedMessage;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.UnknownHostException;
import java.util.function.Consumer;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * UDP-based network bridge for MoppyMessages.
 * 
 * This bridge will utilize multicast addresses to simplify communication
 * to multiple Moppy devices on a Network.  Ideally this should be an effectively zero-conf
 * bridge and work out of the box on any normal network.
 */
public class BridgeUDP extends NetworkBridge {

    private static final int MOPPY_PORT = 30994;
    private InetAddress groupAddress;
    private MulticastSocket socket;
    private Thread listenerThread = null;
    
    public BridgeUDP() throws UnknownHostException {
        groupAddress = InetAddress.getByName("227.2.2.7");
    }
    
    @Override
    public void connect() throws IOException {
        // Create and connect socket
        socket = new MulticastSocket(MOPPY_PORT);
        //socket.connect(groupAddress, MOPPY_PORT);
        socket.joinGroup(groupAddress);
        
        // Create and start listener thread
        UDPListener listener = new UDPListener(socket, this::messageToReceivers);
        listenerThread = new Thread(listener);
        listenerThread.start();
    }

    @Override
    public void sendMessage(MoppyMessage messageToSend) throws IOException {
        if (socket == null /*|| !socket.isConnected()*/) {
            Logger.getLogger(MultiBridge.class.getName()).log(Level.FINE, "UDP Socket null or not connected");
            return; // We're not connected-- just silently fail.
        }
        
        DatagramPacket dgp = new DatagramPacket(messageToSend.getMessageBytes(), 
            messageToSend.getMessageBytes().length,
            groupAddress,
            MOPPY_PORT);
        socket.send(dgp);
    }

    @Override
    public void close() throws IOException {
        try {
            sendMessage(MoppyMessage.SYS_STOP); // Send a stop message before closing to prevent sticking
            socket.leaveGroup(groupAddress);
        } finally {
            // Close socket
            socket.close();
            socket = null;
            // Stop and cleanup listener thread
            listenerThread.interrupt();
            listenerThread = null;
        }
    }

    @Override
    public String getNetworkIdentifier() {
        return String.format("%s:%s", groupAddress.getHostAddress(), MOPPY_PORT);
    }

    private class UDPListener implements Runnable {

        private final Consumer<NetworkReceivedMessage> messageConsumer;
        private final MulticastSocket socket;
        
        public UDPListener(MulticastSocket socket, Consumer<NetworkReceivedMessage> messageConsumer) {
            this.socket = socket;
            this.messageConsumer = messageConsumer;
        }
        
        @Override
        public void run() {
            
            // MoppyMessages can't be longer than 259 bytes (SOM, DEVADDR, SUBADDR, LEN, [0-255 body bytes])
            // Longer messages will be truncated, but we don't care about them anyway
            DatagramPacket bufferPacket = new DatagramPacket(new byte[259], 259);
            byte[] packetData;
            
            while (!socket.isClosed() && !Thread.interrupted()) {
                try {
                    socket.receive(bufferPacket);
                    packetData = bufferPacket.getData();
                    
                    if (packetData.length > 0 && packetData[0] == MoppyMessage.START_BYTE) {
                        NetworkReceivedMessage receivedMessage = MoppyMessageFactory.networkReceivedFromBytes(
                                packetData,
                                BridgeUDP.class.getName(),
                                socket.getInetAddress().getHostAddress(),
                                bufferPacket.getAddress().getHostAddress());
                        messageConsumer.accept(receivedMessage);
                    }
                    
                    //TODO: send status update with DeviceDescriptor for pong??
                    
                } catch (IOException ex) {
                    Logger.getLogger(BridgeUDP.class.getName()).log(Level.WARNING, null, ex);
                }
            }
        }
        
    }
}
