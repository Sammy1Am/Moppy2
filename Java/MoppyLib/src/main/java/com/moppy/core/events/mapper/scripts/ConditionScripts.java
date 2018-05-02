package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum ConditionScripts {

    ALL_EVENTS("All Events", "true"),
    CHANNELS_ONE_TO_FOUR("Channels 1-4", "c>=1 && c<=4"),
    ONLY_SUPPORTED_NOTES("Only Supported Notes", "n>22 && n<67");

    private final String displayName;
    private final String script;

    private ConditionScripts(String d, String s) {
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
