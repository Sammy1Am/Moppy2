package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum NoteScripts {
    
    STRAIGHT_THROUGH("n"),
    IGNORE_OUT_OF_RANGE("(n>22 && n<67) ? n : null"),
    FORCE_INTO_RANGE("((n-22)%48)+22");
    
    private final String script;       

    private NoteScripts(String s) {
        script = s;
    }

    @Override
    public String toString() {
       return this.script;
    }
}
