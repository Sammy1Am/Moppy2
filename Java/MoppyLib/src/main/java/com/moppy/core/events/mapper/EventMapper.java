package com.moppy.core.events.mapper;

import com.moppy.core.comms.MoppyMessage;

/**
 * Interface that defines an EventMapper.  Used for mapping 
 * input events (e.g. MIDI events) to MoppyMessages.
 * @param <INPUT_EVENT> Type of input event to be mapped to MoppyMessages
 */
public interface EventMapper<INPUT_EVENT> {
    public MoppyMessage mapEvent(INPUT_EVENT event);
}
