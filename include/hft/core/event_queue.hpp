
#pragma once

#include <queue>
#include "hft/core/event.hpp"

namespace hft {

class EventQueue {
private:
    std::queue<Event> events;

public:
    void push(const Event& event) {
        events.push(event);
    }

    bool empty() const {
        return events.empty();
    }

    std::size_t size() const {
        return events.size();
    }

    Event pop() {
        Event event = events.front();
        events.pop();
        return event;
    }
};

}
