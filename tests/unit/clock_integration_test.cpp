
#include <gtest/gtest.h>

#include "hft/core/simulated_clock.hpp"
#include "hft/core/system_clock.hpp"
#include "hft/execution/simulator/matching_engine.hpp"

using namespace hft;

TEST(ClockTest, SimulatedClockStartsAtSpecifiedTime) {
    SimulatedClock clock(1000);

    EXPECT_EQ(clock.now(), 1000);
}

TEST(ClockTest, SimulatedClockCanAdvance) {
    SimulatedClock clock(1000);

    clock.advance(500);

    EXPECT_EQ(clock.now(), 1500);
}

TEST(ClockTest, SimulatedClockCanSetTime) {
    SimulatedClock clock(1000);

    clock.setTime(5000);

    EXPECT_EQ(clock.now(), 5000);
}

TEST(ClockTest, SystemClockReturnsNonZeroTimestamp) {
    SystemClock clock;

    EXPECT_GT(clock.now(), 0);
}

TEST(ClockIntegrationTest, AcceptanceReportUsesInjectedClock) {
    OrderBook book;
    SimulatedClock clock(1000);
    MatchingEngine engine(book, clock);

    Order order{1, Side::BUY, 100, 10, 50};

    engine.processOrder(order);

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 1);
    EXPECT_EQ(reports[0].type, ExecutionReportType::ACCEPTED);
    EXPECT_EQ(reports[0].timestamp, 1000);
}

TEST(ClockIntegrationTest, FillReportsUseInjectedClock) {
    OrderBook book;
    SimulatedClock clock(2000);
    MatchingEngine engine(book, clock);

    Order sellOrder{1, Side::SELL, 100, 10, 50};
    Order buyOrder{2, Side::BUY, 100, 10, 60};

    engine.processOrder(sellOrder);
    engine.processOrder(buyOrder);

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 4);

    EXPECT_EQ(reports[2].timestamp, 2000);
    EXPECT_EQ(reports[3].timestamp, 2000);
}

TEST(ClockIntegrationTest, CancellationUsesCurrentClockTime) {
    OrderBook book;
    SimulatedClock clock(3000);
    MatchingEngine engine(book, clock);

    Order order{1, Side::BUY, 100, 10, 50};

    engine.processOrder(order);

    clock.advance(500);

    ASSERT_TRUE(engine.cancelOrder(1));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 2);
    EXPECT_EQ(reports[1].type, ExecutionReportType::CANCELLED);
    EXPECT_EQ(reports[1].timestamp, 3500);
}
