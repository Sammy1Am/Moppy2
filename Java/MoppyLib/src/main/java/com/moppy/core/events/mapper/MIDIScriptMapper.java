package com.moppy.core.events.mapper;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.MoppyMessageFactory;
import com.moppy.core.events.mapper.scripts.ConditionScripts;
import com.moppy.core.events.mapper.scripts.DeviceAddressScripts;
import com.moppy.core.events.mapper.scripts.NoteScripts;
import com.moppy.core.events.mapper.scripts.SubAddressScripts;
import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.HostAccess;
import org.graalvm.polyglot.PolyglotException;
import org.graalvm.polyglot.Value;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.function.Function;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.SysexMessage;

/**
 * Maps events based on JavaScript expressions.
 */
public class MIDIScriptMapper extends MIDIEventMapper {

    //private static final ScriptEngine SCRIPT_ENGINE = new ScriptEngineManager().getEngineByName("graal.js");
    private static final Context SCRIPT_CONTEXT = Context.newBuilder("js").allowHostAccess(HostAccess.ALL).build();
    private final Value localBindings = SCRIPT_CONTEXT.getBindings("js");

    private final HashMap<Integer, Integer> currentNotes = new HashMap<>(); // Sub-Address to Note map
    private int nextOpenRoundRobinSubAddress = 0;

    private String conditionScript = ConditionScripts.ALL_EVENTS.toString();
    private String deviceAddressScript = DeviceAddressScripts.DEVICE_ONE.toString();
    private String subAddressScript = SubAddressScripts.SUB_ADDRESS_PER_CHANNEL.toString();
    private String noteScript = NoteScripts.STRAIGHT_THROUGH.toString();

    private void setDefaultBindings() {
        // Set some sane defaults for testing
        localBindings.putMember("c", 0);
        localBindings.putMember("n", 60);
        localBindings.putMember("v", 127);
        localBindings.putMember("midiCommand", ShortMessage.NOTE_ON);
    }

    private void setLowBindings() {
        // Set some sane defaults for testing
        localBindings.putMember("c", 0);
        localBindings.putMember("n", 0);
        localBindings.putMember("v", 1);
    }

    private void setHighBindings() {
        // Set some sane defaults for testing
        localBindings.putMember("c", 15);
        localBindings.putMember("n", 127);
        localBindings.putMember("v", 127);
    }

    public MIDIScriptMapper() {
        setDefaultBindings();

        // Setup utility functions
        localBindings.putMember("nextRoundRobinSubAddress", (Function<Integer, Integer>)this::nextRoundRobinSubAddress);
    }

    @Override
    public MoppyMessage mapEvent(MidiMessage event) {
        if (event instanceof ShortMessage) {
            ShortMessage midiMessage = (ShortMessage)event;

            // Bind message variables
            localBindings.putMember("c", midiMessage.getChannel());
            localBindings.putMember("midiCommand", midiMessage.getCommand());
            if (midiMessage.getCommand() == ShortMessage.NOTE_ON || midiMessage.getCommand() == ShortMessage.NOTE_OFF) {
                localBindings.putMember("n", midiMessage.getData1());
                localBindings.putMember("v", midiMessage.getData2());
            }
            try {

                if (!resolveCondition()) {
                    return null; // If the condition doesn't match, just give up
                }

                switch (midiMessage.getCommand()) {
                    case ShortMessage.NOTE_ON:
                        if (midiMessage.getData2() == 0) {
                            // For zero-velocity notes, turn the note off
                            return MoppyMessageFactory.deviceStopNote(resolveDeviceId(), resolveSubAddress(), resolveNote());
                        }
                        return MoppyMessageFactory.devicePlayNote(resolveDeviceId(), resolveSubAddress(), resolveNote(), (byte)midiMessage.getData2());
                    case ShortMessage.NOTE_OFF:
                        return MoppyMessageFactory.deviceStopNote(resolveDeviceId(), resolveSubAddress(), resolveNote());
                    case ShortMessage.PITCH_BEND:
                        /*
                        MIDI pitch bends are weird.  This next line converts the two 7-bit pitch bend values
                        into a single value, and then subtracts 8192 (the "no bend" number).  The result is a signed
                        short where 0 = no bend, and with a range of -8192 to 8191
                        */
                        short pitchBend = (short)(((midiMessage.getData2() << 7) + midiMessage.getData1()) - 8192);
                        return MoppyMessageFactory.devicePitchBend(resolveDeviceId(), resolveSubAddress(), pitchBend);
                }
            } catch (PolyglotException | ClassCastException ex) {
                Logger.getLogger(MIDIScriptMapper.class.getName()).log(Level.WARNING, null, ex);
            }
        } else if (event instanceof SysexMessage) {
            SysexMessage sysexMessage = (SysexMessage)event;
            // Check to make sure it's a "Moppy" System exclusive message
            if (sysexMessage.getData()[0] == MoppyMessage.START_BYTE) {
                // Convert the system exclusive message directly into a MoppyMessage
                // NO VALIDATION IS DONE HERE (so the system exclusive messages must be
                // well-formatted)
                return MoppyMessageFactory.fromBytes(Arrays.copyOf(sysexMessage.getData(), sysexMessage.getData().length-1)); // Remove trailing 0xf7 byte
            }
        }
        return null; // We don't know how to handle this event
    }

    private boolean resolveCondition() throws PolyglotException {
        return SCRIPT_CONTEXT.eval("js", conditionScript).asBoolean();
    }

    private byte resolveDeviceId() throws PolyglotException {
        return SCRIPT_CONTEXT.eval("js", deviceAddressScript).asByte();
        //return ((Number)SCRIPT_ENGINE.eval(deviceAddressScript, bindings)).byteValue();
    }

    private byte resolveSubAddress() throws PolyglotException {
        return SCRIPT_CONTEXT.eval("js", subAddressScript).asByte();
        //return ((Number)SCRIPT_ENGINE.eval(subAddressScript, bindings)).byteValue();
    }

    private byte resolveNote() throws PolyglotException {
        return SCRIPT_CONTEXT.eval("js", noteScript).asByte();
        //return ((Number)SCRIPT_ENGINE.eval(noteScript, bindings)).byteValue();
    }

    public String getConditionScript() {
        return conditionScript;
    }

    public String getDeviceAddressScript() {
        return deviceAddressScript;
    }

    public String getSubAddressScript() {
        return subAddressScript;
    }

    public String getNoteScript() {
        return noteScript;
    }

    // Setters will attempt to evaluate the new script before setting it.

    public void setConditionScript(String conditionScript) throws PolyglotException, NullPointerException {
        setDefaultBindings();
        boolean checkOutput = SCRIPT_CONTEXT.eval("js", conditionScript).asBoolean();
        setLowBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", conditionScript).asBoolean();
        setHighBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", conditionScript).asBoolean();
        this.conditionScript = conditionScript;
    }

    public void setDeviceAddressScript(String deviceAddressScript) throws PolyglotException, NullPointerException {
        setDefaultBindings();
        byte checkOutput = SCRIPT_CONTEXT.eval("js", deviceAddressScript).asByte();
        setLowBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", deviceAddressScript).asByte();
        setHighBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", deviceAddressScript).asByte();
        this.deviceAddressScript = deviceAddressScript;
    }

    public void setSubAddressScript(String subAddressScript) throws PolyglotException, NullPointerException {
        setDefaultBindings();
        byte checkOutput = SCRIPT_CONTEXT.eval("js", subAddressScript).asByte();
        setLowBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", subAddressScript).asByte();
        setHighBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", subAddressScript).asByte();
        this.subAddressScript = subAddressScript;
    }

    public void setNoteScript(String noteScript) throws PolyglotException, NullPointerException {
        setDefaultBindings();
        byte checkOutput = SCRIPT_CONTEXT.eval("js", noteScript).asByte();
        setLowBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", noteScript).asByte();
        setHighBindings();
        checkOutput = SCRIPT_CONTEXT.eval("js", noteScript).asByte();
        this.noteScript = noteScript;
    }

    ////
    // Utility functions accessible from scripts
    ////

    private int nextRoundRobinSubAddress(Integer numberOfChannels) {
        int incomingNote = localBindings.getMember("n").asInt(); // NOTE: Could be leftover binding from previously handled message! Confirm this event is a NOTE_ON or NOTE_OFF event!
        if ((localBindings.getMember("midiCommand").asInt()) == ShortMessage.NOTE_ON) {
            // If this is a note on event, increment the sub address to the next available
            nextOpenRoundRobinSubAddress = nextOpenRoundRobinSubAddress%numberOfChannels+1;
            // Set the note in the map so we know what to turn off the next time we get a note off event
            currentNotes.put(nextOpenRoundRobinSubAddress, incomingNote);
            return nextOpenRoundRobinSubAddress;
        } else if ((localBindings.getMember("midiCommand").asInt()) == ShortMessage.NOTE_OFF
                && currentNotes.containsValue(incomingNote)) {
            int subAddressToTurnOff = numberOfChannels+1; // Initialize to an address outside the rotation

            // If we're currently playing this note somewhere, figure out where it is, and then remove it from the map
            for (Entry<Integer, Integer> e : currentNotes.entrySet()) {
                if (e.getValue().equals(incomingNote)) {
                    subAddressToTurnOff = e.getKey();
                    break;
                }
            }
            currentNotes.remove(subAddressToTurnOff);
            return subAddressToTurnOff;
        }
        return numberOfChannels+1; // If it's not a note on or off, push it outside the range (so hopefully it's ignored)
    }
}
