package com.moppy.control.config;

import com.moppy.core.events.mapper.scripts.ConditionScripts;
import com.moppy.core.events.mapper.scripts.DeviceAddressScripts;
import com.moppy.core.events.mapper.scripts.NoteScripts;
import com.moppy.core.events.mapper.scripts.SubAddressScripts;
import java.awt.Dimension;
import java.util.ArrayList;
import java.util.List;
import lombok.Data;

/**
 * Stores configuration options in a JSON-serializable format.
 */
@Data
public class MoppyConfig {
    // Sequencer
    private String fileLoadDirectory = ".";
    private boolean autoReset = false;
    private Dimension mainWindowSize = new Dimension(1024, 600);
    private int mainWindowDividerPosition = 300;

    // Mappers
    private List<MIDIScriptMapperConfig> mapperConfigs = new ArrayList<>();

    public MoppyConfig() {
        // Add a new default mapper (will be overridden if the user loads preferences)
        mapperConfigs.add(new MIDIScriptMapperConfig());
    }

    @Data
    public static class MIDIScriptMapperConfig {
        private String conditionChoice = ConditionScripts.ALL_EVENTS.displayName();
        private String conditionCustomScript = ConditionScripts.ALL_EVENTS.toString();
        private String deviceAddressChoice = DeviceAddressScripts.DEVICE_ONE.displayName();
        private String deviceAddressCustomScript = DeviceAddressScripts.DEVICE_ONE.toString();
        private String subAddressChoice = SubAddressScripts.SUB_ADDRESS_PER_CHANNEL.displayName();
        private String subAddressCustomScript = SubAddressScripts.SUB_ADDRESS_PER_CHANNEL.toString();
        private String noteChoice = NoteScripts.STRAIGHT_THROUGH.displayName();
        private String noteCustomScript = NoteScripts.STRAIGHT_THROUGH.toString();
    }
}
