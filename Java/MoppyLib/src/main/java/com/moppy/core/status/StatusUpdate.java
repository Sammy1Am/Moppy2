package com.moppy.core.status;

import java.util.Optional;
import javax.sound.midi.Sequence;

/**
 * A status update sent from a sequencer or network.
 */
public class StatusUpdate {

    private final StatusType type;
    private final Optional<Object> data;

    private StatusUpdate(StatusType type, Optional<Object> data) {
        this.type = type;
        this.data = data;
    }

    public StatusType getType() {
        return type;
    }

    public Optional<Object> getData() {
        return data;
    }

    // Sequencer statuses

    public static StatusUpdate SEQUENCE_START = new StatusUpdate(StatusType.SEQUENCE_START, Optional.empty());
    public static StatusUpdate SEQUENCE_PAUSE = new StatusUpdate(StatusType.SEQUENCE_PAUSE, Optional.empty());
    public static StatusUpdate SEQUENCE_STOPPED = new StatusUpdate(StatusType.SEQUENCE_STOPPED, Optional.of(true)); // Always reset when stopped

    public static StatusUpdate sequenceEnd(boolean doReset) {
        return new StatusUpdate(StatusType.SEQUENCE_END, Optional.of(doReset));
    }
    public static StatusUpdate tempoChange(float tempo) {
        return new StatusUpdate(StatusType.SEQUENCE_TEMPO_CHANGE, Optional.of(tempo));
    }
    public static StatusUpdate sequenceLoaded(Sequence sequence) {
        return new StatusUpdate(StatusType.SEQUENCE_LOAD, Optional.of(sequence));
    }

    // Network statuses

    public static StatusUpdate NET_STATUS_CHANGED = new StatusUpdate(StatusType.NET_STATUS_CHANGED, Optional.empty());
    public static StatusUpdate NET_DEVICES_CHANGED = new StatusUpdate(StatusType.NET_DEVICES_CHANGED, Optional.empty());
}
