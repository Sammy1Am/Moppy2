package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum SubAddressScripts {
    
    SUB_ADDRESS_PER_CHANNEL("c+1");
    
    private final String script;       

    private SubAddressScripts(String s) {
        script = s;
    }

    @Override
    public String toString() {
       return this.script;
    }
}
