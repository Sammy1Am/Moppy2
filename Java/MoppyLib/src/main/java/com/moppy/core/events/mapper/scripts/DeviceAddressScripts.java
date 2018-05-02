package com.moppy.core.events.mapper.scripts;

/**
 *
 */
public enum DeviceAddressScripts {

    DEVICE_ONE("Device 1", "1"),
    ONE_DEVICE_PER_CHANNEL("One Device per Channel", "c+1");

    private final String displayName;
    private final String script;

    private DeviceAddressScripts(String d, String s) {
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
