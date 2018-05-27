package com.moppy.core.midi;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.bridge.NetworkBridge;
import com.moppy.core.status.StatusSender;
import com.moppy.core.events.mapper.MapperCollection;
import com.moppy.core.events.postprocessor.MessagePostProcessor;
import java.io.IOException;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.Receiver;

/**
 * Connects a MIDI Transmitter to a Moppy NetworkBridge
 */
public class MoppyMIDIReceiverSender extends StatusSender implements Receiver {

    private final MapperCollection<MidiMessage> mappers;
    private final MessagePostProcessor postProcessor;

    public MoppyMIDIReceiverSender(MapperCollection<MidiMessage> mapperCollection, MessagePostProcessor postProcessor, NetworkBridge netBridge) throws IOException {
        super(netBridge);
        this.mappers = mapperCollection;
        this.postProcessor = postProcessor;
    }

    @Override
    public void send(MidiMessage message, long timeStamp) {
        Set<MoppyMessage> messagesToSend = mappers.mapEvent(message);
        messagesToSend = messagesToSend.stream().map(postProcessor::postProcess).collect(Collectors.toSet());

        messagesToSend.forEach((messageToSend) -> {
            try {
                networkBridge.sendMessage(messageToSend);
            } catch (IOException ex) {
                Logger.getLogger(MoppyMIDIReceiverSender.class.getName()).log(Level.WARNING, null, ex);
            }
        });
    }

    @Override
    public void close() {
        //TODO: Need to decide if it's best to control connect / disconnect from netBridge with ReceiverBridge,
        // or just control those directly on bridge instance
    }

}
