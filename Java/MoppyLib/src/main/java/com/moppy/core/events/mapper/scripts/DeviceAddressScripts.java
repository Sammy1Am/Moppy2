package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum DeviceAddressScripts {
    
    DEVICE_ONE("1"),
    ONE_DEVICE_PER_CHANNEL("c+1");
    
    private final String script;       

    private DeviceAddressScripts(String s) {
        script = s;
    }

    @Override
    public String toString() {
       return this.script;
    }
}
