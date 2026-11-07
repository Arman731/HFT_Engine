
#include <gtest/gtest.h>

#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

#include "hft/core/event_logger.hpp"
#include "hft/core/event_log_reader.hpp"
#include "hft/core/event_replayer.hpp"

#include "hft/core/event_engine.hpp"
#include "hft/core/event_handler.hpp"

using namespace hft;

class EventLoggingTest : public ::testing::Test {
protected:
    const std::string filename = "test_event_log.csv";

    void SetUp() override {
        std::remove(filename.c_str());
    }

    void TearDown() override {
        std::remove(filename.c_str());
    }
};

TEST_F(EventLoggingTest, LoggerWritesEventToCSV) {
    {
        EventLogger logger(filename);

        logger.logEvent(
            Event{EventType::ORDER_ACCEPTED, 100, 1}
        );
    }

    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());

    std::string header;
    std::string record;

    std::getline(file, header);
    std::getline(file, record);

    EXPECT_EQ(header, "event_type,timestamp,order_id");
    EXPECT_EQ(record, "ORDER_ACCEPTED,100,1");
}

TEST_F(EventLoggingTest, LoggerAppendsWithoutDuplicatingHeader) {
    {
        EventLogger logger(filename);
        logger.logEvent(
            Event{EventType::ORDER_ACCEPTED, 100, 1}
        );
    }

    {
        EventLogger logger(filename);
        logger.logEvent(
            Event{EventType::TRADE_EXECUTED, 200, 2}
        );
    }

    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    ASSERT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[0], "event_type,timestamp,order_id");
    EXPECT_EQ(lines[1], "ORDER_ACCEPTED,100,1");
    EXPECT_EQ(lines[2], "TRADE_EXECUTED,200,2");
}

TEST_F(EventLoggingTest, ReaderReconstructsEvents) {
    {
        EventLogger logger(filename);

        logger.logEvent(
            Event{EventType::ORDER_ACCEPTED, 100, 1}
        );

        logger.logEvent(
            Event{EventType::TRADE_EXECUTED, 200, 2}
        );
    }

    EventLogReader reader(filename);

    ASSERT_TRUE(reader.hasNext());

    Event first = reader.readNextEvent();

    EXPECT_EQ(first.type, EventType::ORDER_ACCEPTED);
    EXPECT_EQ(first.timestamp, 100);
    EXPECT_EQ(first.orderId, 1);

    ASSERT_TRUE(reader.hasNext());

    Event second = reader.readNextEvent();

    EXPECT_EQ(second.type, EventType::TRADE_EXECUTED);
    EXPECT_EQ(second.timestamp, 200);
    EXPECT_EQ(second.orderId, 2);

    EXPECT_FALSE(reader.hasNext());
}

TEST_F(EventLoggingTest, ReaderRejectsInvalidHeader) {
    {
        std::ofstream file(filename);
        ASSERT_TRUE(file.is_open());
        file << "invalid,header\n";
    }

    EXPECT_THROW(EventLogReader reader(filename), std::runtime_error);
}

TEST_F(EventLoggingTest, ReaderRejectsMalformedRecord) {
    {
        std::ofstream file(filename);
        ASSERT_TRUE(file.is_open());

        file << "event_type,timestamp,order_id\n";
        file << "ORDER_ACCEPTED,invalid,1\n";
    }

    EventLogReader reader(filename);

    EXPECT_THROW(reader.readNextEvent(), std::runtime_error);
}

TEST_F(EventLoggingTest, EventEngineLogsPublishedEvents) {
    {
        EventLogger logger(filename);
        EventEngine engine;

        engine.setLogger(logger);

        engine.publish(
            Event{EventType::ORDER_ACCEPTED, 100, 1}
        );

        engine.publish(
            Event{EventType::TRADE_EXECUTED, 200, 2}
        );
    }

    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    ASSERT_EQ(lines.size(), 3);
    EXPECT_EQ(lines[1], "ORDER_ACCEPTED,100,1");
    EXPECT_EQ(lines[2], "TRADE_EXECUTED,200,2");
}

TEST_F(EventLoggingTest, ReplayerFeedsEventsIntoEngine) {
    {
        EventLogger logger(filename);

        logger.logEvent(
            Event{EventType::ORDER_ACCEPTED, 100, 1}
        );

        logger.logEvent(
            Event{EventType::TRADE_EXECUTED, 200, 2}
        );
    }

    EventLogReader reader(filename);
    EventEngine engine;

    class ReplayHandler : public EventHandler {
    public:
        std::vector<Event> received;

        void onEvent(const Event& event) override {
            received.push_back(event);
        }
    };

    ReplayHandler handler;

    engine.subscribe(EventType::ORDER_ACCEPTED, handler);
    engine.subscribe(EventType::TRADE_EXECUTED, handler);

    EventReplayer replayer(reader, engine);

    EXPECT_EQ(replayer.replayAll(), 2);
    EXPECT_EQ(engine.pendingEvents(), 2);

    engine.processAllEvents();

    ASSERT_EQ(handler.received.size(), 2);
    EXPECT_EQ(handler.received[0].type, EventType::ORDER_ACCEPTED);
    EXPECT_EQ(handler.received[1].type, EventType::TRADE_EXECUTED);
}
