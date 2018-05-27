package com.moppy.core.events.postprocessor;

import com.moppy.core.comms.MoppyMessage;

/**
 * Used for transforming MoppyMessages after mapping (intially used for volume control)
 */
public interface MessagePostProcessor {
    public MoppyMessage postProcess(MoppyMessage message);

    /**
     * Does no post-processing on messages.
     */
    public static MessagePostProcessor PASS_THROUGH = new MessagePostProcessor() {
        @Override
        public MoppyMessage postProcess(MoppyMessage message) {
            return message;
        }
    };
}
