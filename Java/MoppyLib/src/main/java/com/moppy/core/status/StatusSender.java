/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.status;

import com.moppy.core.comms.MoppyMessage;
import com.moppy.core.comms.bridge.NetworkBridge;
import com.moppy.core.events.mapper.BaseEventMapper;
import com.moppy.core.status.StatusConsumer;
import com.moppy.core.status.StatusUpdate;
import java.io.IOException;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Sends local control / sequencer events to the network bridge.
 */
public abstract class StatusSender implements StatusConsumer {
    public final NetworkBridge networkBridge;
    
    public StatusSender(NetworkBridge netBridge) {
        this.networkBridge = netBridge;
    }

    @Override
    public void receiveUpdate(StatusUpdate update) {
        List<MoppyMessage> messagesToSend = BaseEventMapper.mapStatusUpdate(update);

        messagesToSend.forEach((messageToSend) -> {
            try {
                networkBridge.sendMessage(messageToSend);
            } catch (IOException ex) {
                Logger.getLogger(StatusSender.class.getName()).log(Level.WARNING, String.format("Could not send %s", messageToSend), ex);
            }
        });
    }
}
