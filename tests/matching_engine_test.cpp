
#include <gtest/gtest.h>

#include "core/order.hpp"
#include "matching/matching_engine.hpp"

TEST(MatchingEngine, BuyOrderMatchesSellOrder) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    // First, add a SELL order to the book.
    hft::Order sellOrder{
        1001,
        hft::Side::SELL,
        15040,
        10,
        1000000
    };

    engine.processOrder(sellOrder);

    // Now submit a BUY order at the same price.
    hft::Order buyOrder{
        1002,
        hft::Side::BUY,
        15040,
        10,
        1000001
    };

    engine.processOrder(buyOrder);

    // Check that exactly one trade occurred.
    const auto& trades = engine.getTrades();

    ASSERT_EQ(trades.size(), 1);

    // Verify the trade details.
    EXPECT_EQ(trades[0].buyOrderId, 1002);
    EXPECT_EQ(trades[0].sellOrderId, 1001);
    EXPECT_EQ(trades[0].price, 15040);
    EXPECT_EQ(trades[0].quantity, 10);

    // Both orders should be completely filled.
    EXPECT_EQ(book.getBestAsk(), 0);
    EXPECT_EQ(book.getBestBid(), 0);
}
