package com.moppy.core.events.mapper;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.MoppyMessageFactory;

import java.util.Arrays;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.SysexMessage;

/**
 * Maps MIDI events to MoppyMessages
 */
public abstract class MIDIEventMapper implements EventMapper<MidiMessage> {

    /**
     * Maps MIDI channels 1-16 to subAddresses 1-16 for the given deviceAddress
     * @param deviceAddress Device address to map MIDI notes to
     * @return
     */
    public static MIDIEventMapper defaultMapper(byte deviceAddress) {
        return new MIDIEventMapper() {

            private final byte targetAddress = deviceAddress;

            @Override
            public MoppyMessage mapEvent(MidiMessage event) {
                if (event instanceof ShortMessage) {
                    ShortMessage midiMessage = (ShortMessage) event;
                    switch (midiMessage.getCommand()) {
                        case ShortMessage.NOTE_ON:
                            if (midiMessage.getData2() == 0) {
                                // For zero-velocity notes, turn the note off
                                return MoppyMessageFactory.deviceStopNote(targetAddress, (byte)(midiMessage.getChannel()+1), (byte)midiMessage.getData1());
                            }
                            return MoppyMessageFactory.devicePlayNote(targetAddress, (byte)(midiMessage.getChannel()+1), (byte)midiMessage.getData1(), (byte)midiMessage.getData2());
                        case ShortMessage.NOTE_OFF:
                            return MoppyMessageFactory.deviceStopNote(targetAddress, (byte)(midiMessage.getChannel()+1), (byte)midiMessage.getData1());
                        case ShortMessage.PITCH_BEND:
                            /*
                            MIDI pitch bends are weird.  This next line converts the two 7-bit pitch bend values
                            into a single value, and then subtracts 8192 (the "no bend" number).  The result is a signed
                            short where 0 = no bend, and with a range of -8192 to 8191
                            */
                            short pitchBend = (short)((((midiMessage.getData2() & 0xff) << 7) + midiMessage.getData1() & 0xff) - 8192);
                            return MoppyMessageFactory.devicePitchBend(targetAddress, (byte)(midiMessage.getChannel()+1), pitchBend);
                    }
                } else if (event instanceof SysexMessage) {
                    SysexMessage sysexMessage = (SysexMessage) event;
                    // Check to make sure it's a "Moppy" System exclusive message
                    if (sysexMessage.getData()[0] == MoppyMessage.START_BYTE) {
                        // Convert the system exclusive message directly into a MoppyMessage
                        // NO VALIDATION IS DONE HERE (so the system exclusive messages must be
                        // well-formatted)
                        return MoppyMessageFactory
                                .fromBytes(Arrays.copyOf(sysexMessage.getData(), sysexMessage.getData().length - 1)); // Remove
                                                                                                                      // trailing
                                                                                                                      // 0xf7
                                                                                                                      // byte
                    }
                }
                return null; // We don't know how to handle this event
            }
        };
    }
}
