/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.events.mapper;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.status.StatusUpdate;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Maps StatusUpdates to network events.
 */
public class BaseEventMapper {
    public static List<MoppyMessage> mapStatusUpdate(StatusUpdate statusUpdate) {
        switch (statusUpdate.getType()) {
            case SEQUENCE_START:
                return Arrays.asList(MoppyMessage.SYS_START);
            case SEQUENCE_PAUSE:
                return Arrays.asList(MoppyMessage.SYS_STOP);
            case SEQUENCE_END:
                return Arrays.asList(MoppyMessage.SYS_STOP, MoppyMessage.SYS_RESET);
            default:
                return Collections.emptyList();
        }
    }
}
