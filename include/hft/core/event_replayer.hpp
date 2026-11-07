
#pragma once

#include <cstddef>

#include "hft/core/event_log_reader.hpp"
#include "hft/core/event_engine.hpp"

namespace hft {

class EventReplayer {
private:
    EventLogReader& reader;
    EventEngine& engine;

public:
    EventReplayer(EventLogReader& eventReader,
                  EventEngine& eventEngine)
        : reader(eventReader),
          engine(eventEngine) {}

    std::size_t replayAll() {
        std::size_t replayed = 0;

        while (reader.hasNext()) {
            Event event = reader.readNextEvent();
            engine.publishReplayed(event);
            ++replayed;
        }

        return replayed;
    }
};

}
