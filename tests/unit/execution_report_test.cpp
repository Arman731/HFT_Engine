
#include <gtest/gtest.h>

#include "hft/execution/simulator/matching_engine.hpp"

using namespace hft;

class ExecutionReportTest : public ::testing::Test {
protected:
    OrderBook book;
    MatchingEngine engine{book};

    Order makeOrder(
        OrderId id,
        Side side,
        Price price,
        Quantity quantity
    ) {
        return {id, side, price, quantity, 100};
    }
};

TEST_F(ExecutionReportTest, AcceptedOrderGeneratesAcceptanceReport) {
    engine.processOrder(makeOrder(1, Side::BUY, 100, 10));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 1);

    EXPECT_EQ(reports[0].orderId, 1);
    EXPECT_EQ(reports[0].type, ExecutionReportType::ACCEPTED);
    EXPECT_EQ(reports[0].cumulativeFilledQuantity, 0);
    EXPECT_EQ(reports[0].remainingQuantity, 10);
}

TEST_F(ExecutionReportTest, FullyFilledOrderGeneratesFillReports) {
    engine.processOrder(makeOrder(1, Side::SELL, 100, 10));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 10));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 4);

    EXPECT_EQ(reports[2].type, ExecutionReportType::FILLED);
    EXPECT_EQ(reports[3].type, ExecutionReportType::FILLED);

    EXPECT_EQ(reports[2].lastExecutedQuantity, 10);
    EXPECT_EQ(reports[2].cumulativeFilledQuantity, 10);
    EXPECT_EQ(reports[2].remainingQuantity, 0);

    EXPECT_EQ(reports[3].lastExecutedQuantity, 10);
    EXPECT_EQ(reports[3].cumulativeFilledQuantity, 10);
    EXPECT_EQ(reports[3].remainingQuantity, 0);
}


TEST_F(ExecutionReportTest, PartialFillGeneratesCorrectReport) {
    engine.processOrder(makeOrder(1, Side::SELL, 100, 10));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 4));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 4);

    // Incoming BUY order is completely filled.
    EXPECT_EQ(reports[2].orderId, 2);
    EXPECT_EQ(reports[2].type, ExecutionReportType::FILLED);
    EXPECT_EQ(reports[2].lastExecutedQuantity, 4);
    EXPECT_EQ(reports[2].cumulativeFilledQuantity, 4);
    EXPECT_EQ(reports[2].remainingQuantity, 0);

    // Resting SELL order is partially filled.
    EXPECT_EQ(reports[3].orderId, 1);
    EXPECT_EQ(reports[3].type, ExecutionReportType::PARTIALLY_FILLED);
    EXPECT_EQ(reports[3].lastExecutedQuantity, 4);
    EXPECT_EQ(reports[3].cumulativeFilledQuantity, 4);
    EXPECT_EQ(reports[3].remainingQuantity, 6);
}


TEST_F(ExecutionReportTest, CancelledOrderGeneratesCancellationReport) {
    engine.processOrder(makeOrder(1, Side::BUY, 100, 10));

    ASSERT_TRUE(engine.cancelOrder(1));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 2);

    EXPECT_EQ(reports[1].type, ExecutionReportType::CANCELLED);
    EXPECT_EQ(reports[1].cumulativeFilledQuantity, 0);
    EXPECT_EQ(reports[1].remainingQuantity, 10);
}

TEST_F(ExecutionReportTest, InvalidOrderGeneratesRejectionReport) {
    engine.processOrder(makeOrder(1, Side::BUY, -100, 10));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 1);

    EXPECT_EQ(reports[0].type, ExecutionReportType::REJECTED);
    EXPECT_EQ(reports[0].orderId, 1);
}

TEST_F(ExecutionReportTest, ReportsPreserveExecutionPrice) {
    engine.processOrder(makeOrder(1, Side::SELL, 99, 5));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 5));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 4);

    EXPECT_EQ(reports[2].executionPrice, 99);
    EXPECT_EQ(reports[3].executionPrice, 99);
}

TEST_F(ExecutionReportTest, DuplicateOrderGeneratesRejectionReport) {
    engine.processOrder(makeOrder(1, Side::BUY, 100, 10));
    engine.processOrder(makeOrder(1, Side::BUY, 100, 5));

    const auto& reports = engine.getExecutionReports();

    ASSERT_EQ(reports.size(), 2);

    EXPECT_EQ(reports[0].type, ExecutionReportType::ACCEPTED);
    EXPECT_EQ(reports[1].type, ExecutionReportType::REJECTED);
}
