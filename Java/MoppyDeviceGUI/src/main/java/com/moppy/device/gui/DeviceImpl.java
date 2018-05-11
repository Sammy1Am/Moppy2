package com.moppy.device.gui;

import com.jsyn.JSyn;
import com.jsyn.Synthesizer;
import com.jsyn.unitgen.LineOut;
import com.jsyn.unitgen.Pan;
import com.moppy.core.comms.MoppyMessageFactory;
import com.moppy.core.comms.NetworkMessageConsumer;
import com.moppy.core.comms.NetworkReceivedMessage;
import com.moppy.core.comms.bridge.BridgeUDP;
import com.moppy.core.comms.bridge.NetworkBridge;
import com.moppy.core.device.MoppyDevice;
import com.moppy.core.util.Notes;
import java.io.Closeable;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Sample device implementation.
 */
public class DeviceImpl extends MoppyDevice implements Closeable, NetworkMessageConsumer {

    private final byte deviceAddress = 0x01;
    private final byte numberOfDevices = 0x08;

    NetworkBridge network;

    private final SimFloppyDrive[] simDrives = new SimFloppyDrive[8];
    Synthesizer synth = JSyn.createSynthesizer();
    LineOut lout = new LineOut();

    public DeviceImpl() throws UnknownHostException {
        network = new BridgeUDP();
        network.registerMessageReceiver(this);

        synth.add(lout);

        Pan pan = new Pan();
        pan.pan.set(0.0);
        pan.output.connect(0,lout.input,0);
        pan.output.connect(1,lout.input,1);

        for (int d=0;d<8;d++){
            SimFloppyDrive sd = new SimFloppyDrive();
            simDrives[d] = sd;
            synth.add(sd.so);
            sd.so.output.connect(pan.input);
        }
    }

    public void connect() throws IOException {
        network.connect();
        synth.start();
        lout.start();
    }

    @Override
    public void close() throws IOException {
        network.close();
        lout.stop();
        synth.stop();
    }

    @Override
    public boolean matchesAddress(byte deviceAddress, byte subAddress) {
        return deviceAddress == this.deviceAddress && subAddress <= this.numberOfDevices;
    }

    @Override
    public void acceptNetworkMessage(NetworkReceivedMessage networkMessage) {
        this.handleMessage(networkMessage);
    }

    // System handlers

    @Override
    public void gotSystemPing() {
        try {
            network.sendMessage(MoppyMessageFactory.systemPong(deviceAddress, (byte)1, numberOfDevices));
        } catch (IOException ex) {
            Logger.getLogger(DeviceImpl.class.getName()).log(Level.WARNING, null, ex);
        }
    }

    @Override
    public void systemReset() {
        resetDrives();
    }

    // Device handlers

    @Override
    public void devicePlayNote(byte deviceAddress, byte subAddress, byte noteNumber) {
        simDrives[subAddress - 1].playFrequency(Notes.FREQUENCIES[noteNumber]);
    }

    @Override
    public void deviceStopNote(byte deviceAddress, byte subAddress, byte noteNumber) {
        simDrives[subAddress - 1].stopFrequency();
    }

    @Override
    public void deviceReset(byte deviceAddress, byte subAddress) {
        resetDrives();
    }

    private void resetDrives() {
        for (int d=0;d<8;d++){
            simDrives[d].resetDrive();
        }
    }

}
