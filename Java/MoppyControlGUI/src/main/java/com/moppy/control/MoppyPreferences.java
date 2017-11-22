package com.moppy.control;

import java.util.prefs.Preferences;

/**
 * 
 */
public class MoppyPreferences {
    private static final Preferences MOPPY_PREFS = Preferences.userNodeForPackage(MoppyPreferences.class);
    
    public static final String LOAD_FILE_DIR = "loadFileDir";
    
    public static String getString(String preferenceKey, String defaultPreference) {
        return MOPPY_PREFS.get(preferenceKey, defaultPreference);
    }
    
    public static void setString(String preferenceKey, String preferenceValue) {
        MOPPY_PREFS.put(preferenceKey, preferenceValue);
    }
}
