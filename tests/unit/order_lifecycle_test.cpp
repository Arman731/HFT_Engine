
#include <gtest/gtest.h>

#include "hft/core/order_lifecycle.hpp"
#include "hft/execution/simulator/matching_engine.hpp"

using namespace hft;

class OrderLifecycleTest : public ::testing::Test {
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

TEST_F(OrderLifecycleTest, NewOrderStartsAsAccepted) {
    auto order = makeOrder(1, Side::BUY, 100, 10);

    engine.processOrder(order);

    const auto* lifecycle = engine.getOrderLifecycle(1);

    ASSERT_NE(lifecycle, nullptr);
    EXPECT_EQ(lifecycle->status, OrderLifecycleStatus::ACCEPTED);
    EXPECT_EQ(lifecycle->originalQuantity, 10);
    EXPECT_EQ(lifecycle->filledQuantity, 0);
    EXPECT_EQ(lifecycle->remainingQuantity, 10);
}

TEST_F(OrderLifecycleTest, FullyFilledOrderHasFilledStatus) {
    engine.processOrder(makeOrder(1, Side::SELL, 100, 10));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 10));

    const auto* lifecycle = engine.getOrderLifecycle(2);

    ASSERT_NE(lifecycle, nullptr);
    EXPECT_EQ(lifecycle->status, OrderLifecycleStatus::FILLED);
    EXPECT_EQ(lifecycle->filledQuantity, 10);
    EXPECT_EQ(lifecycle->remainingQuantity, 0);
}

TEST_F(OrderLifecycleTest, RestingOrderCanBecomePartiallyFilled) {
    engine.processOrder(makeOrder(1, Side::SELL, 100, 10));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 4));

    const auto* lifecycle = engine.getOrderLifecycle(1);

    ASSERT_NE(lifecycle, nullptr);
    EXPECT_EQ(lifecycle->status, OrderLifecycleStatus::PARTIALLY_FILLED);
    EXPECT_EQ(lifecycle->filledQuantity, 4);
    EXPECT_EQ(lifecycle->remainingQuantity, 6);
}

TEST_F(OrderLifecycleTest, IncomingOrderCanBePartiallyFilled) {
    engine.processOrder(makeOrder(1, Side::SELL, 100, 10));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 15));

    const auto* lifecycle = engine.getOrderLifecycle(2);

    ASSERT_NE(lifecycle, nullptr);
    EXPECT_EQ(lifecycle->status, OrderLifecycleStatus::PARTIALLY_FILLED);
    EXPECT_EQ(lifecycle->filledQuantity, 10);
    EXPECT_EQ(lifecycle->remainingQuantity, 5);
}

TEST_F(OrderLifecycleTest, PartiallyFilledOrderCanBeCancelled) {
    engine.processOrder(makeOrder(1, Side::SELL, 100, 10));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 4));

    EXPECT_TRUE(engine.cancelOrder(1));

    const auto* lifecycle = engine.getOrderLifecycle(1);

    ASSERT_NE(lifecycle, nullptr);
    EXPECT_EQ(lifecycle->status, OrderLifecycleStatus::CANCELLED);
    EXPECT_EQ(lifecycle->filledQuantity, 4);
    EXPECT_EQ(lifecycle->remainingQuantity, 6);
}

TEST_F(OrderLifecycleTest, InvalidOrderIsMarkedRejected) {
    auto result = engine.processOrder(
        makeOrder(1, Side::BUY, -100, 10)
    );

    EXPECT_EQ(result.status, OrderStatus::INVALID_PRICE);

    const auto* lifecycle = engine.getOrderLifecycle(1);

    ASSERT_NE(lifecycle, nullptr);
    EXPECT_EQ(lifecycle->status, OrderLifecycleStatus::REJECTED);
}

TEST_F(OrderLifecycleTest, FilledOrderCannotBeCancelled) {
    engine.processOrder(makeOrder(1, Side::SELL, 100, 10));
    engine.processOrder(makeOrder(2, Side::BUY, 100, 10));

    EXPECT_FALSE(engine.cancelOrder(2));

    const auto* lifecycle = engine.getOrderLifecycle(2);

    ASSERT_NE(lifecycle, nullptr);
    EXPECT_EQ(lifecycle->status, OrderLifecycleStatus::FILLED);
}

TEST_F(OrderLifecycleTest, UnknownOrderHasNoLifecycle) {
    EXPECT_EQ(engine.getOrderLifecycle(999), nullptr);
}
