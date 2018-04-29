package com.moppy.control;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.moppy.control.config.MoppyConfig;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.prefs.Preferences;

/**
 *
 */
public class MoppyPreferences {
    private static final ObjectMapper MAPPER = new ObjectMapper();
    private static final Preferences MOPPY_PREFS = Preferences.userNodeForPackage(MoppyPreferences.class);
    private static final String CONFIG_KEY = "moppyConfig";

    private static final MoppyConfig CONFIG;

    static {
        MoppyConfig parsedConfig;
        try {
            parsedConfig = MAPPER.readValue(MOPPY_PREFS.get(CONFIG_KEY, "{}"), MoppyConfig.class);
        } catch (IOException ex) {
            parsedConfig = new MoppyConfig();
            Logger.getLogger(MoppyPreferences.class.getName()).log(Level.WARNING, "Failed to parse configuration-- using default", ex);
        }
        CONFIG = parsedConfig;
    }

    public static MoppyConfig getConfiguration() {
        return CONFIG;
    }

    public static void saveConfiguration() {
        try {
            MOPPY_PREFS.put(CONFIG_KEY, MAPPER.writeValueAsString(CONFIG));
        } catch (JsonProcessingException ex) {
            Logger.getLogger(MoppyPreferences.class.getName()).log(Level.WARNING, "Failed to save preferences!", ex);
        }
    }
}
