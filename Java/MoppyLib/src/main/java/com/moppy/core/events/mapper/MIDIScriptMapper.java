package com.moppy.core.events.mapper;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.MoppyMessageFactory;
import com.moppy.core.events.mapper.scripts.ConditionScripts;
import com.moppy.core.events.mapper.scripts.DeviceAddressScripts;
import com.moppy.core.events.mapper.scripts.NoteScripts;
import com.moppy.core.events.mapper.scripts.SubAddressScripts;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.function.Function;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.script.Bindings;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;

/**
 * Maps events based on JavaScript expressions.
 */
public class MIDIScriptMapper extends MIDIEventMapper {

    private static final ScriptEngine SCRIPT_ENGINE = new ScriptEngineManager().getEngineByName("javascript");
    private final static Bindings TEST_BINDINGS;
    private final Bindings localBindings = SCRIPT_ENGINE.createBindings();

    private final HashMap<Integer, Integer> currentNotes = new HashMap<>(); // Sub-Address to Note map
    private int nextOpenRoundRobinSubAddress = 0;

    private String conditionScript = ConditionScripts.ALL_EVENTS.toString();
    private String deviceAddressScript = DeviceAddressScripts.DEVICE_ONE.toString();
    private String subAddressScript = SubAddressScripts.SUB_ADDRESS_PER_CHANNEL.toString();
    private String noteScript = NoteScripts.STRAIGHT_THROUGH.toString();

    static {
        TEST_BINDINGS = SCRIPT_ENGINE.createBindings();
        TEST_BINDINGS.put("c", 0);
        TEST_BINDINGS.put("n", 60);
        TEST_BINDINGS.put("v", 127);
    }

    public MIDIScriptMapper() {
        // Set some sane defaults for testing
        localBindings.put("c", 0);
        localBindings.put("n", 60);
        localBindings.put("v", 127);
        localBindings.put("midiCommand", ShortMessage.NOTE_ON);

        // Setup utility functions
        localBindings.put("nextRoundRobinSubAddress", (Function<Integer, Integer>)this::nextRoundRobinSubAddress);
    }

    @Override
    public MoppyMessage mapEvent(MidiMessage event) {
        if (event instanceof ShortMessage) {
            ShortMessage midiMessage = (ShortMessage) event;

            // Bind message variables
            localBindings.put("c", midiMessage.getChannel());
            localBindings.put("midiCommand", midiMessage.getCommand());
            if (midiMessage.getCommand() == ShortMessage.NOTE_ON || midiMessage.getCommand() == ShortMessage.NOTE_OFF) {
                localBindings.put("n", midiMessage.getData1());
                localBindings.put("v", midiMessage.getData2());
            }
            try {

                if (!resolveCondition(localBindings)) {
                    return null; // If the condition doesn't match, just give up
                }

                switch (midiMessage.getCommand()) {
                    case ShortMessage.NOTE_ON:
                        if (midiMessage.getData2() == 0) {
                            // For zero-velocity notes, turn the note off
                            return MoppyMessageFactory.deviceStopNote(resolveDeviceId(localBindings), resolveSubAddress(localBindings), resolveNote(localBindings));
                        }
                        return MoppyMessageFactory.devicePlayNote(resolveDeviceId(localBindings), resolveSubAddress(localBindings), resolveNote(localBindings), (byte)midiMessage.getData2());
                    case ShortMessage.NOTE_OFF:
                        return MoppyMessageFactory.deviceStopNote(resolveDeviceId(localBindings), resolveSubAddress(localBindings), resolveNote(localBindings));
                    case ShortMessage.PITCH_BEND:
                        /*
                        MIDI pitch bends are weird.  This next line converts the two 7-bit pitch bend values
                        into a single value, and then subtracts 8192 (the "no bend" number).  The result is a signed
                        short where 0 = no bend, and with a range of -8192 to 8191
                        */
                        short pitchBend = (short)(((midiMessage.getData2() << 7) + midiMessage.getData1()) - 8192);
                        return MoppyMessageFactory.devicePitchBend(resolveDeviceId(localBindings), resolveSubAddress(localBindings), pitchBend);
                }
            } catch (ScriptException | ClassCastException ex) {
                 Logger.getLogger(MIDIScriptMapper.class.getName()).log(Level.WARNING, null, ex);
            }
        }
        return null; // We don't know how to handle this event
    }

    private boolean resolveCondition(Bindings bindings) throws ScriptException {
        return (boolean)SCRIPT_ENGINE.eval(conditionScript, bindings);
    }

    private byte resolveDeviceId(Bindings bindings) throws ScriptException {
        return ((Number)SCRIPT_ENGINE.eval(deviceAddressScript, bindings)).byteValue();
    }
    private byte resolveSubAddress(Bindings bindings) throws ScriptException {
        return ((Number)SCRIPT_ENGINE.eval(subAddressScript, bindings)).byteValue();
    }
    private byte resolveNote(Bindings bindings) throws ScriptException {
        return ((Number)SCRIPT_ENGINE.eval(noteScript, bindings)).byteValue();
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

    public void setConditionScript(String conditionScript) throws ScriptException {
        boolean checkOutput = (boolean)SCRIPT_ENGINE.eval(conditionScript, localBindings);
        this.conditionScript = conditionScript;
    }

    public void setDeviceAddressScript(String deviceAddressScript) throws ScriptException {
        byte checkOutput = ((Number)SCRIPT_ENGINE.eval(deviceAddressScript, localBindings)).byteValue();
        this.deviceAddressScript = deviceAddressScript;
    }

    public void setSubAddressScript(String subAddressScript) throws ScriptException {
        byte checkOutput = ((Number)SCRIPT_ENGINE.eval(subAddressScript, localBindings)).byteValue();
        this.subAddressScript = subAddressScript;
    }

    public void setNoteScript(String noteScript) throws ScriptException {
        byte checkOutput = ((Number)SCRIPT_ENGINE.eval(noteScript, localBindings)).byteValue();
        this.noteScript = noteScript;
    }

    ////
    // Utility functions accessible from scripts
    ////

    private int nextRoundRobinSubAddress(Integer numberOfChannels) {
        int incomingNote = (int)localBindings.get("n"); // NOTE: Could be leftover binding from previously handled message!  Confirm this event is a NOTE_ON or NOTE_OFF event!
        if (((int)localBindings.get("midiCommand")) == ShortMessage.NOTE_ON) {
            // If this is a note on event, increment the sub address to the next available
            nextOpenRoundRobinSubAddress = nextOpenRoundRobinSubAddress%8+1;
            // Set the note in the map so we know what to turn off the next time we get a note off event
            currentNotes.put(nextOpenRoundRobinSubAddress, incomingNote);
            return nextOpenRoundRobinSubAddress;
        } else if (((int)localBindings.get("midiCommand")) == ShortMessage.NOTE_OFF
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
