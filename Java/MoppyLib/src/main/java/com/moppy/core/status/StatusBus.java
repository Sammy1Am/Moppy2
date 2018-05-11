/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.moppy.core.status;

import java.util.HashSet;
import java.util.Set;

/**
 * Acts as a bus to send StatusUpdates to multiple StatusConsumers.
 */
public class StatusBus implements StatusConsumer {
    private final Set<StatusConsumer> consumers = new HashSet<>();

    public void registerConsumer(StatusConsumer consumer) {
        consumers.add(consumer);
    }

    public void deregisterConsumer(StatusConsumer consumer) {
        consumers.remove(consumer);
    }

    @Override
    public void receiveUpdate(StatusUpdate update) {
        consumers.forEach(consumer -> consumer.receiveUpdate(update));
    }
}
