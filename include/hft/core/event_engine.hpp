
#pragma once

#include <unordered_map>
#include <vector>
#include <cstddef>

#include "hft/core/event_queue.hpp"
#include "hft/core/event_handler.hpp"

#include "hft/core/event_logger.hpp"

namespace hft {

class EventEngine {
private:
    EventQueue eventQueue;

    EventLogger* logger = nullptr;

    // Non-owning pointers.
    // Handlers must remain alive while subscribed or during dispatch.
    std::unordered_map<EventType, std::vector<EventHandler*>> handlers;

public:

    void setLogger(EventLogger& eventLogger) {
        logger = &eventLogger;
    }

    void publish(const Event& event) {
        if (logger != nullptr) {
            logger->logEvent(event);
        }

        eventQueue.push(event);
    }

    void publishReplayed(const Event& event) {
        eventQueue.push(event);
    }

    void subscribe(EventType type, EventHandler& handler) {
        auto& subscribers = handlers[type];

        for (EventHandler* existingHandler : subscribers) {
            if (existingHandler == &handler) {
                return;
            }
        }

        subscribers.push_back(&handler);
    }

    void unsubscribe(EventType type, EventHandler& handler) {
        auto it = handlers.find(type);

        if (it == handlers.end()) {
            return;
        }

        auto& subscribers = it->second;

        for (auto handlerIt = subscribers.begin();
             handlerIt != subscribers.end();) {

            if (*handlerIt == &handler) {
                handlerIt = subscribers.erase(handlerIt);
            } else {
                ++handlerIt;
            }
        }

        if (subscribers.empty()) {
            handlers.erase(it);
        }
    }

    void processNextEvent() {
        if (eventQueue.empty()) {
            return;
        }

        Event event = eventQueue.pop();

        auto it = handlers.find(event.type);

        if (it != handlers.end()) {
            auto subscribers = it->second;

            for (EventHandler* handler : subscribers) {
                handler->onEvent(event);
            }
        }
    }

    void processAllEvents() {
        while (!eventQueue.empty()) {
            processNextEvent();
        }
    }

    std::size_t processEvents(std::size_t maxEvents) {
        std::size_t processed = 0;

        while (!eventQueue.empty() && processed < maxEvents) {
            processNextEvent();
            ++processed;
        }

        return processed;
    }

    bool hasEvents() const {
        return !eventQueue.empty();
    }

    Event getNextEvent() {
        return eventQueue.pop();
    }

    std::size_t pendingEvents() const {
        return eventQueue.size();
    }
};

}
