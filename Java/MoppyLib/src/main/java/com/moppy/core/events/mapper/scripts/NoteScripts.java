package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum NoteScripts {

    STRAIGHT_THROUGH("Straight Through", "n"),
    FORCE_INTO_RANGE("Force into Range", "((n-24)%48)+24");

    private final String displayName;
    private final String script;

    private NoteScripts(String d, String s) {
        displayName = d;
        script = s;
    }

    @Override
    public String toString() {
       return this.script;
    }

    public String displayName() {
        return this.displayName;
    }
}
