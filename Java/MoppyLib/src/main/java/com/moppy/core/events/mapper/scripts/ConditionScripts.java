package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum ConditionScripts {
    
    ALL_EVENTS("true"),
    CHANNELS_ONE_TO_FOUR("c>=1 && c<=4"),
    ONLY_SUPPORTED_NOTES("n>22 && n<67");
    
    private final String script;       

    private ConditionScripts(String s) {
        script = s;
    }

    @Override
    public String toString() {
       return this.script;
    }
}
