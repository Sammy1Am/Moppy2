package com.moppy.core.status;

public enum StatusType {
        // Sequencer statuses
        SEQUENCE_LOAD,
        SEQUENCE_START,
        SEQUENCE_PAUSE,
        SEQUENCE_END,
        SEQUENCE_TEMPO_CHANGE,

        // Network statuses
        NET_STATUS_CHANGED, // A NetworkBridge has either connected or disconnected
        NET_DEVICES_CHANGED // A device has either become available or unavailable on the network
    }
