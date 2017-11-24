package com.moppy.core.events.mapper;

import com.moppy.core.comms.MoppyMessage;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Collection of EventMappers.
 * @param <INPUT_EVENT> Type of input event to be mapped to MoppyMessages
 */
public class MapperCollection<INPUT_EVENT> {

    private final List<EventMapper<INPUT_EVENT>> mappers = new ArrayList<>();

    public void addMapper(EventMapper<INPUT_EVENT> mapperToAdd) {
        mappers.add(mapperToAdd);
    }

    public void removeMapper(EventMapper<INPUT_EVENT> mapperToRemove) {
        mappers.remove(mapperToRemove);
    }

    public void clearMappers() {
        mappers.clear();
    }

    public List<EventMapper<INPUT_EVENT>> getMappers() {
        return mappers;
    }

    public Set<MoppyMessage> mapEvent(INPUT_EVENT event) {
        return mappers.stream()
                .map(mapper -> mapper.mapEvent(event))
                .filter(message -> message != null) // Remove any null messages (the mapper didn't know how to handle those)
                .collect(Collectors.toSet());
    }
}
