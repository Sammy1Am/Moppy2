package com.moppy.core.midi;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.bridge.NetworkBridge;
import com.moppy.core.status.StatusSender;
import com.moppy.core.events.mapper.MapperCollection;
import com.moppy.core.events.postprocessor.MessagePostProcessor;
import java.io.IOException;
import java.util.Optional;
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
    private Optional<Receiver> midiThru = Optional.empty();

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

        // If a midiThru receiver has been specified, forward the message.
        if (midiThru.isPresent()) {
            midiThru.get().send(message, timeStamp);
        }
    }

    @Override
    public void close() {
        //TODO: Need to decide if it's best to control connect / disconnect from netBridge with ReceiverBridge,
        // or just control those directly on bridge instance
    }

    /**
     * Sets a midi receiver to receive all MIDI messages.
     * @param midiThru Receiver for MIDI messages, or null to disable MIDI Throughput
     */
    public void setMidiThru(Receiver midiThru) {
        this.midiThru = midiThru != null ? Optional.of(midiThru) : Optional.empty();
    }

}
