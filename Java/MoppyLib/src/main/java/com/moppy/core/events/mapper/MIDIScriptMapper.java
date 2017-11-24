package com.moppy.core.events.mapper;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.MoppyMessageFactory;
import com.moppy.core.events.mapper.scripts.ConditionScripts;
import com.moppy.core.events.mapper.scripts.DeviceAddressScripts;
import com.moppy.core.events.mapper.scripts.NoteScripts;
import com.moppy.core.events.mapper.scripts.SubAddressScripts;
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

    private String conditionScript = ConditionScripts.ALL_EVENTS.toString();
    private String deviceAddressScript = DeviceAddressScripts.DEVICE_ONE.toString();
    private String subAddressScript = SubAddressScripts.SUB_ADDRESS_PER_CHANNEL.toString();
    private String noteScript = NoteScripts.STRAIGHT_THROUGH.toString();

    static {
        TEST_BINDINGS = SCRIPT_ENGINE.createBindings();
        TEST_BINDINGS.put("c", 0);
        TEST_BINDINGS.put("n", 60);
    }

    @Override
    public MoppyMessage mapEvent(MidiMessage event) {
        if (event instanceof ShortMessage) {
            ShortMessage midiMessage = (ShortMessage) event;

            // Bind message variables
            Bindings eventBindings = SCRIPT_ENGINE.createBindings();
            eventBindings.put("c", midiMessage.getChannel());
            if (midiMessage.getCommand() == ShortMessage.NOTE_ON || midiMessage.getCommand() == ShortMessage.NOTE_OFF) {
                eventBindings.put("n", midiMessage.getData1());
            }
            try {

                if (!resolveCondition(eventBindings)) {
                    return null; // If the condition doesn't match, just give up
                }

                switch (midiMessage.getCommand()) {
                    case ShortMessage.NOTE_ON:
                        if (midiMessage.getData2() == 0) {
                            // For zero-velocity notes, turn the note off
                            return MoppyMessageFactory.deviceStopNote(resolveDeviceId(eventBindings), resolveSubAddress(eventBindings), resolveNote(eventBindings));
                        }
                        return MoppyMessageFactory.devicePlayNote(resolveDeviceId(eventBindings), resolveSubAddress(eventBindings), resolveNote(eventBindings));
                    case ShortMessage.NOTE_OFF:
                        return MoppyMessageFactory.deviceStopNote(resolveDeviceId(eventBindings), resolveSubAddress(eventBindings), resolveNote(eventBindings));
                    case ShortMessage.PITCH_BEND:
                        /*
                        MIDI pitch bends are weird.  This next line converts the two 7-bit pitch bend values
                        into a single value, and then subtracts 8192 (the "no bend" number).  The result is a signed
                        short where 0 = no bend, and with a range of -8192 to 8191
                        */
                        short pitchBend = (short)((((midiMessage.getData2() & 0xff) << 7) + midiMessage.getData1() & 0xff) - 8192);
                        return MoppyMessageFactory.devicePitchBend(resolveDeviceId(eventBindings), resolveSubAddress(eventBindings), pitchBend);
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
        boolean checkOutput = (boolean)SCRIPT_ENGINE.eval(conditionScript, TEST_BINDINGS);
        this.conditionScript = conditionScript;
    }

    public void setDeviceAddressScript(String deviceAddressScript) throws ScriptException {
        byte checkOutput = ((Number)SCRIPT_ENGINE.eval(deviceAddressScript, TEST_BINDINGS)).byteValue();
        this.deviceAddressScript = deviceAddressScript;
    }

    public void setSubAddressScript(String subAddressScript) throws ScriptException {
        byte checkOutput = ((Number)SCRIPT_ENGINE.eval(subAddressScript, TEST_BINDINGS)).byteValue();
        this.subAddressScript = subAddressScript;
    }

    public void setNoteScript(String noteScript) throws ScriptException {
        byte checkOutput = ((Number)SCRIPT_ENGINE.eval(noteScript, TEST_BINDINGS)).byteValue();
        this.noteScript = noteScript;
    }

}
