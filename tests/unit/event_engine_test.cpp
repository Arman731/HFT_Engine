
#include <gtest/gtest.h>

#include "hft/core/event.hpp"
#include "hft/core/event_queue.hpp"
#include "hft/core/event_engine.hpp"
#include "hft/core/event_handler.hpp"

using namespace hft;

TEST(EventQueueTest, StartsEmpty) {
    EventQueue queue;

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST(EventQueueTest, PushIncreasesSize) {
    EventQueue queue;

    Event event{EventType::ORDER_ACCEPTED, 100, 1};
    queue.push(event);

    EXPECT_EQ(queue.size(), 1);
    EXPECT_FALSE(queue.empty());
}

TEST(EventQueueTest, PopReturnsOldestEvent) {
    EventQueue queue;

    Event first{EventType::ORDER_ACCEPTED, 100, 1};
    Event second{EventType::ORDER_REJECTED, 200, 2};

    queue.push(first);
    queue.push(second);

    Event result = queue.pop();

    EXPECT_EQ(result.type, EventType::ORDER_ACCEPTED);
    EXPECT_EQ(result.timestamp, 100);
    EXPECT_EQ(result.orderId, 1);
    EXPECT_EQ(queue.size(), 1);
}

TEST(EventEngineTest, PublishAddsEvent) {
    EventEngine engine;

    Event event{EventType::ORDER_ACCEPTED, 100, 1};
    engine.publish(event);

    EXPECT_TRUE(engine.hasEvents());
    EXPECT_EQ(engine.pendingEvents(), 1);
}

TEST(EventEngineTest, RetrievesPublishedEvent) {
    EventEngine engine;

    Event event{EventType::TRADE_EXECUTED, 200, 5};
    engine.publish(event);

    Event result = engine.getNextEvent();

    EXPECT_EQ(result.type, EventType::TRADE_EXECUTED);
    EXPECT_EQ(result.timestamp, 200);
    EXPECT_EQ(result.orderId, 5);
    EXPECT_FALSE(engine.hasEvents());
}


class TestEventHandler : public EventHandler {
public:
    int eventsReceived = 0;
    Event lastEvent{EventType::ORDER_ACCEPTED, 0, 0};

    void onEvent(const Event& event) override {
        eventsReceived++;
        lastEvent = event;
    }
};

TEST(EventEngineTest, SubscriberReceivesMatchingEvent) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);

    Event event{EventType::ORDER_ACCEPTED, 100, 1};
    engine.publish(event);
    engine.processNextEvent();

    EXPECT_EQ(handler.eventsReceived, 1);
    EXPECT_EQ(handler.lastEvent.orderId, 1);
}

TEST(EventEngineTest, HandlerDoesNotReceiveUnrelatedEvent) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);

    Event event{EventType::TRADE_EXECUTED, 200, 5};
    engine.publish(event);
    engine.processNextEvent();

    EXPECT_EQ(handler.eventsReceived, 0);
}

TEST(EventEngineTest, MultipleHandlersReceiveSameEvent) {
    EventEngine engine;
    TestEventHandler firstHandler;
    TestEventHandler secondHandler;

    engine.subscribe(EventType::TRADE_EXECUTED, firstHandler);
    engine.subscribe(EventType::TRADE_EXECUTED, secondHandler);

    Event event{EventType::TRADE_EXECUTED, 300, 10};
    engine.publish(event);
    engine.processNextEvent();

    EXPECT_EQ(firstHandler.eventsReceived, 1);
    EXPECT_EQ(secondHandler.eventsReceived, 1);
}

TEST(EventEngineTest, ProcessAllEventsDispatchesEverything) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);

    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});
    engine.publish(Event{EventType::ORDER_ACCEPTED, 200, 2});
    engine.publish(Event{EventType::ORDER_ACCEPTED, 300, 3});

    engine.processAllEvents();

    EXPECT_EQ(handler.eventsReceived, 3);
    EXPECT_FALSE(engine.hasEvents());
    EXPECT_EQ(engine.pendingEvents(), 0);
}

TEST(EventEngineTest, ProcessingEmptyQueueDoesNotCrash) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);

    engine.processNextEvent();
    engine.processAllEvents();

    EXPECT_EQ(handler.eventsReceived, 0);
}


class PublishingEventHandler : public EventHandler {
private:
    EventEngine& engine;

public:
    explicit PublishingEventHandler(EventEngine& eventEngine)
        : engine(eventEngine) {}

    void onEvent(const Event& event) override {
        if (event.type == EventType::ORDER_ACCEPTED) {
            engine.publish(
                Event{EventType::TRADE_EXECUTED, 200, 2}
            );
        }
    }
};

class SelfUnsubscribingHandler : public EventHandler {
private:
    EventEngine& engine;
    EventType subscribedType;

public:
    int eventsReceived = 0;

    SelfUnsubscribingHandler(
        EventEngine& eventEngine,
        EventType type
    ) : engine(eventEngine), subscribedType(type) {}

    void onEvent(const Event& event) override {
        eventsReceived++;
        engine.unsubscribe(subscribedType, *this);
    }
};

TEST(EventEngineTest, DuplicateSubscriptionIsIgnored) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);
    engine.subscribe(EventType::ORDER_ACCEPTED, handler);

    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});
    engine.processNextEvent();

    EXPECT_EQ(handler.eventsReceived, 1);
}

TEST(EventEngineTest, UnsubscribeStopsEventDelivery) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);
    engine.unsubscribe(EventType::ORDER_ACCEPTED, handler);

    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});
    engine.processNextEvent();

    EXPECT_EQ(handler.eventsReceived, 0);
}

TEST(EventEngineTest, UnsubscribeUnregisteredHandlerDoesNothing) {
    EventEngine engine;
    TestEventHandler handler;

    engine.unsubscribe(EventType::ORDER_ACCEPTED, handler);

    EXPECT_FALSE(engine.hasEvents());
}

TEST(EventEngineTest, UnsubscribeOnlyAffectsSpecifiedEventType) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);
    engine.subscribe(EventType::TRADE_EXECUTED, handler);

    engine.unsubscribe(EventType::ORDER_ACCEPTED, handler);

    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});
    engine.publish(Event{EventType::TRADE_EXECUTED, 200, 2});

    engine.processAllEvents();

    EXPECT_EQ(handler.eventsReceived, 1);
    EXPECT_EQ(handler.lastEvent.type, EventType::TRADE_EXECUTED);
}

TEST(EventEngineTest, ProcessesEventsPublishedByHandlers) {
    EventEngine engine;

    PublishingEventHandler publisher(engine);
    TestEventHandler tradeHandler;

    engine.subscribe(EventType::ORDER_ACCEPTED, publisher);
    engine.subscribe(EventType::TRADE_EXECUTED, tradeHandler);

    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});

    engine.processAllEvents();

    EXPECT_EQ(tradeHandler.eventsReceived, 1);
    EXPECT_EQ(tradeHandler.lastEvent.orderId, 2);
    EXPECT_FALSE(engine.hasEvents());
}

TEST(EventEngineTest, ProcessEventsRespectsMaximumLimit) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);

    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});
    engine.publish(Event{EventType::ORDER_ACCEPTED, 200, 2});
    engine.publish(Event{EventType::ORDER_ACCEPTED, 300, 3});

    std::size_t processed = engine.processEvents(2);

    EXPECT_EQ(processed, 2);
    EXPECT_EQ(handler.eventsReceived, 2);
    EXPECT_EQ(engine.pendingEvents(), 1);
}

TEST(EventEngineTest, ProcessEventsWithZeroLimitProcessesNothing) {
    EventEngine engine;
    TestEventHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);
    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});

    std::size_t processed = engine.processEvents(0);

    EXPECT_EQ(processed, 0);
    EXPECT_EQ(handler.eventsReceived, 0);
    EXPECT_EQ(engine.pendingEvents(), 1);
}

TEST(EventEngineTest, SelfUnsubscriptionDuringDispatchIsSafe) {
    EventEngine engine;

    SelfUnsubscribingHandler selfHandler(
        engine, EventType::ORDER_ACCEPTED
    );

    TestEventHandler otherHandler;

    engine.subscribe(EventType::ORDER_ACCEPTED, selfHandler);
    engine.subscribe(EventType::ORDER_ACCEPTED, otherHandler);

    engine.publish(Event{EventType::ORDER_ACCEPTED, 100, 1});
    engine.publish(Event{EventType::ORDER_ACCEPTED, 200, 2});

    engine.processAllEvents();

    EXPECT_EQ(selfHandler.eventsReceived, 1);
    EXPECT_EQ(otherHandler.eventsReceived, 2);
}