package com.moppy.core.status;

/**
 * Implements methods that are called by the sequencer or networkbridges to allow
 * reporting status to the UI.
 */
public interface StatusConsumer {
    public void receiveUpdate(StatusUpdate update);
}
