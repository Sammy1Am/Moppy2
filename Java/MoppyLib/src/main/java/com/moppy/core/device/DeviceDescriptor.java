package com.moppy.core.device;

import lombok.Builder;
import lombok.EqualsAndHashCode;

/**
 * Class to provide meta information for a device on the network.
 */
@EqualsAndHashCode
@Builder
public class DeviceDescriptor {
    private String networkAddress;
    private byte deviceAddress;
    private byte minSubAddress;
    private byte maxSubAddress;

    public String getNetworkAddress() {
        return networkAddress;
    }

    public void setNetworkAddress(String networkAddress) {
        this.networkAddress = networkAddress;
    }

    public byte getDeviceAddress() {
        return deviceAddress;
    }

    public void setDeviceAddress(byte deviceAddress) {
        this.deviceAddress = deviceAddress;
    }

    public byte getMinSubAddress() {
        return minSubAddress;
    }

    public void setMinSubAddress(byte minSubAddress) {
        this.minSubAddress = minSubAddress;
    }

    public byte getMaxSubAddress() {
        return maxSubAddress;
    }

    public void setMaxSubAddress(byte maxSubAddress) {
        this.maxSubAddress = maxSubAddress;
    }



}
