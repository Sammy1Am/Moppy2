package com.moppy.core.midi;

import com.moppy.core.status.StatusBus;
import com.moppy.core.status.StatusUpdate;
import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MetaEventListener;
import javax.sound.midi.MetaMessage;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.Sequence;
import javax.sound.midi.Sequencer;
import lombok.Setter;

/**
 * Wrapper around the Java MIDI sequencer for playing MIDI files.
 *
 * Additionally provides feedback to listeners about the current state of the sequencer.
 */
public class MoppyMIDISequencer implements MetaEventListener, Closeable {
    private static final Logger LOG = Logger.getLogger(MoppyMIDISequencer.class.getName());
    private final Sequencer seq;
    private final StatusBus statusBus;
    @Setter
    private boolean autoReset = false;

    public MoppyMIDISequencer(StatusBus statusBus, MoppyMIDIReceiverSender receiverSender) throws MidiUnavailableException {
        this.statusBus = statusBus;
        this.statusBus.registerConsumer(receiverSender); // Register receiverSender to send seq messages to network
        seq = MidiSystem.getSequencer(false);
        seq.open();
        seq.getTransmitter().setReceiver(receiverSender);
        seq.addMetaEventListener(this);
    }

    @Override
    public void close() throws IOException {
        seq.close();
    }

    @Override
    public void meta(MetaMessage meta) {
        // Handle tempo changes
        if (meta.getType() == 81){
            int uSecondsPerQN = 0;
            uSecondsPerQN |= meta.getData()[0] & 0xFF;
            uSecondsPerQN <<= 8;
            uSecondsPerQN |= meta.getData()[1] & 0xFF;
            uSecondsPerQN <<= 8;
            uSecondsPerQN |= meta.getData()[2] & 0xFF;

            int newTempo = 60000000/uSecondsPerQN;

            setTempo(newTempo);
        }
        // Handle end-of-track events
        else if (meta.getType() == 47) {
            seq.setTickPosition(0); // Reset sequencer so we can press "play" again right away
            //MrSolidSnake745: Exposing end of sequence event to status consumers
            statusBus.receiveUpdate(StatusUpdate.sequenceEnd(autoReset));
        }
    }

    public void play() {
        seq.start();
        statusBus.receiveUpdate(StatusUpdate.SEQUENCE_START);
    }

    public void pause() {
        seq.stop();
        statusBus.receiveUpdate(StatusUpdate.SEQUENCE_PAUSE);
    }

    public void stop() {
        seq.stop();
        seq.setTickPosition(0);
        statusBus.receiveUpdate(StatusUpdate.SEQUENCE_STOPPED);
    }

    public boolean isPlaying() {
        return seq.isRunning();
    }

    public void loadSequence(File sequenceFile) throws IOException, InvalidMidiDataException {
        if (!sequenceFile.isFile()) {
            throw new IOException(String.format("File '%s' not found, or isn't a file", sequenceFile.getAbsolutePath()));
        }
        Sequence sequenceToLoad = MidiSystem.getSequence(sequenceFile);

        seq.setSequence(sequenceToLoad);
        statusBus.receiveUpdate(StatusUpdate.sequenceLoaded(sequenceToLoad));
        statusBus.receiveUpdate(StatusUpdate.tempoChange(seq.getTempoInBPM()));

        LOG.info(String.format("Loaded sequence with %s tracks at %s BMP", sequenceToLoad.getTracks().length-1, seq.getTempoInBPM())); // -1 for system track?
    }
    
    public boolean isSequenceLoaded() {
        return seq.getSequence() != null;
    }

    public long getSecondsLength(){
        return TimeUnit.SECONDS.convert(seq.getMicrosecondLength(), TimeUnit.MICROSECONDS);
    }

    public long getSecondsPosition(){
        return TimeUnit.SECONDS.convert(seq.getMicrosecondPosition(), TimeUnit.MICROSECONDS);
    }

    public void setSecondsPosition(long seconds){
        seq.setMicrosecondPosition(TimeUnit.SECONDS.toMicros(seconds));
    }

    public void setTempo(float newTempo){
        // Don't set the tempo again if it's not different (prevents some weird loops in GUI)
        if (seq.getTempoInBPM() != newTempo) {
            seq.setTempoInBPM(newTempo);
            statusBus.receiveUpdate(StatusUpdate.tempoChange(newTempo));
            LOG.info(String.format("Tempo changed to %s", newTempo));
        }
    }
}
