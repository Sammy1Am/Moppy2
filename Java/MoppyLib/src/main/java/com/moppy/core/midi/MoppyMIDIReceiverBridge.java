package com.moppy.core.midi;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.bridge.NetworkBridge;
import com.moppy.core.events.mapper.MapperCollection;
import java.io.IOException;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.Receiver;

/**
 * Connects a MIDI Transmitter to a Moppy NetworkBridge
 */
public class MoppyMIDIReceiverBridge implements Receiver {

    private final MapperCollection<MidiMessage> mappers;
    private final NetworkBridge netBridge;
    
    public MoppyMIDIReceiverBridge(MapperCollection<MidiMessage> mapperCollection, NetworkBridge netBridge) throws IOException {
        this.mappers = mapperCollection;
        this.netBridge = netBridge;
    }
    
    @Override
    public void send(MidiMessage message, long timeStamp) {
        Set<MoppyMessage> messagesToSend = mappers.mapEvent(message);

        messagesToSend.forEach((messageToSend) -> {
            try {
                netBridge.sendMessage(messageToSend);
            } catch (IOException ex) {
                Logger.getLogger(MoppyMIDIReceiverBridge.class.getName()).log(Level.WARNING, null, ex);
            }
        });
    }

    @Override
    public void close() {
        //TODO: Need to decide if it's best to control connect / disconnect from netBridge with ReceiverBridge, 
        // or just control those directly on bridge instance
    }
    
}
