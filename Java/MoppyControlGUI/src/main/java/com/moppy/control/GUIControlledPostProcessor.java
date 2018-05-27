package com.moppy.control;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.events.postprocessor.MessagePostProcessor;
import lombok.Setter;

/**
 * PostProcessor hooks for GUI settings to process messages.
 */
public class GUIControlledPostProcessor implements MessagePostProcessor {

    /**
     * If true, will cause MIDI velocities to all be set uniformly based on the
     * velocityMultiplyer, if false velocities will be scaled based on multiplier
     */
    @Setter
    private boolean overrideVelocity = false;
    @Setter
    private double velocityMultiplier = 1;


    @Override
    public MoppyMessage postProcess(MoppyMessage message) {
        if (MoppyMessage.CommandByte.DEV_PLAYNOTE == message.getMessageCommandByte()) {
            if (overrideVelocity) {
                message.getMessageBytes()[6] = (byte)(127*velocityMultiplier);
            } else {
                message.getMessageBytes()[6] = (byte)(message.getMessageBytes()[6]*velocityMultiplier);
            }
        }
        return message;
    }

}
