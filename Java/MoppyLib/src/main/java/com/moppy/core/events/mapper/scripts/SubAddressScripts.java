package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum SubAddressScripts {

    SUB_ADDRESS_PER_CHANNEL("Sub-address per Channel", "c+1"),
    ROUND_ROBIN("Round Robin (8 drives)", "nextRoundRobinSubAddress(8)");

    private final String displayName;
    private final String script;

    private SubAddressScripts(String d, String s) {
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
