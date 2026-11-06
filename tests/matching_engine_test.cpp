
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

TEST(MatchingEngine, MatchesBestPriceFirst) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    // Add two SELL orders at different prices.
    hft::Order sell1{
        2001, hft::Side::SELL, 15050, 5, 1000000
    };

    hft::Order sell2{
        2002, hft::Side::SELL, 15040, 5, 1000001
    };

    engine.processOrder(sell1);
    engine.processOrder(sell2);

    // Incoming BUY order.
    hft::Order buy{
        2003, hft::Side::BUY, 15060, 7, 1000002
    };

    engine.processOrder(buy);

    const auto& trades = engine.getTrades();

    ASSERT_EQ(trades.size(), 2);

    // Cheapest SELL order executes first.
    EXPECT_EQ(trades[0].sellOrderId, 2002);
    EXPECT_EQ(trades[0].price, 15040);
    EXPECT_EQ(trades[0].quantity, 5);

    // Remaining 2 units execute at the next price.
    EXPECT_EQ(trades[1].sellOrderId, 2001);
    EXPECT_EQ(trades[1].price, 15050);
    EXPECT_EQ(trades[1].quantity, 2);
}

TEST(MatchingEngine, MatchesOldestOrderFirst) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order sell1{
        3001, hft::Side::SELL, 15040, 5, 1000000
    };

    hft::Order sell2{
        3002, hft::Side::SELL, 15040, 5, 1000001
    };

    engine.processOrder(sell1);
    engine.processOrder(sell2);

    hft::Order buy{
        3003, hft::Side::BUY, 15040, 7, 1000002
    };

    engine.processOrder(buy);

    const auto& trades = engine.getTrades();

    ASSERT_EQ(trades.size(), 2);

    // First order gets completely filled.
    EXPECT_EQ(trades[0].sellOrderId, 3001);
    EXPECT_EQ(trades[0].quantity, 5);

    // Second order gets partially filled.
    EXPECT_EQ(trades[1].sellOrderId, 3002);
    EXPECT_EQ(trades[1].quantity, 2);

    // Three units remain in the second SELL order.
    hft::Order* remaining = book.getBestAskOrder();

    ASSERT_NE(remaining, nullptr);
    EXPECT_EQ(remaining->id, 3002);
    EXPECT_EQ(remaining->quantity, 3);
}

TEST(MatchingEngine, ExecutesAgainstMultipleOrders) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order sell1{
        4001, hft::Side::SELL, 15040, 3, 1000000
    };

    hft::Order sell2{
        4002, hft::Side::SELL, 15040, 4, 1000001
    };

    hft::Order sell3{
        4003, hft::Side::SELL, 15040, 5, 1000002
    };

    engine.processOrder(sell1);
    engine.processOrder(sell2);
    engine.processOrder(sell3);

    hft::Order buy{
        4004, hft::Side::BUY, 15040, 10, 1000003
    };

    engine.processOrder(buy);

    const auto& trades = engine.getTrades();

    ASSERT_EQ(trades.size(), 3);

    EXPECT_EQ(trades[0].quantity, 3);
    EXPECT_EQ(trades[1].quantity, 4);
    EXPECT_EQ(trades[2].quantity, 3);

    EXPECT_EQ(trades[0].sellOrderId, 4001);
    EXPECT_EQ(trades[1].sellOrderId, 4002);
    EXPECT_EQ(trades[2].sellOrderId, 4003);

    // Two units remain in the third SELL order.
    hft::Order* remaining = book.getBestAskOrder();

    ASSERT_NE(remaining, nullptr);
    EXPECT_EQ(remaining->id, 4003);
    EXPECT_EQ(remaining->quantity, 2);
}

TEST(MatchingEngine, CancelsBuyOrder) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order buy{
        5001, hft::Side::BUY, 15030, 10, 1000000
    };

    engine.processOrder(buy);

    EXPECT_EQ(book.getBestBid(), 15030);

    EXPECT_TRUE(engine.cancelOrder(5001));

    EXPECT_EQ(book.getBestBid(), 0);
}

TEST(MatchingEngine, CancelsSellOrder) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order sell{
        5002, hft::Side::SELL, 15040, 10, 1000000
    };

    engine.processOrder(sell);

    EXPECT_EQ(book.getBestAsk(), 15040);

    EXPECT_TRUE(engine.cancelOrder(5002));

    EXPECT_EQ(book.getBestAsk(), 0);
}

TEST(MatchingEngine, RejectsCancellationOfUnknownOrder) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    EXPECT_FALSE(engine.cancelOrder(9999));
}

TEST(MatchingEngine, CancelsPartiallyFilledOrder) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order sell{
        5003, hft::Side::SELL, 15040, 10, 1000000
    };

    hft::Order buy{
        5004, hft::Side::BUY, 15040, 4, 1000001
    };

    engine.processOrder(sell);
    engine.processOrder(buy);

    // Six units of the SELL order remain.
    hft::Order* remaining = book.getBestAskOrder();

    ASSERT_NE(remaining, nullptr);
    EXPECT_EQ(remaining->quantity, 6);

    EXPECT_TRUE(engine.cancelOrder(5003));

    EXPECT_EQ(book.getBestAsk(), 0);

    // The original execution must remain recorded.
    ASSERT_EQ(engine.getTrades().size(), 1);
    EXPECT_EQ(engine.getTrades()[0].quantity, 4);
}

TEST(MatchingEngine, RejectsCancellationOfFilledOrder) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order sell{
        5005, hft::Side::SELL, 15040, 5, 1000000
    };

    hft::Order buy{
        5006, hft::Side::BUY, 15040, 5, 1000001
    };

    engine.processOrder(sell);
    engine.processOrder(buy);

    ASSERT_EQ(engine.getTrades().size(), 1);

    EXPECT_FALSE(engine.cancelOrder(5005));

    EXPECT_EQ(book.getBestAsk(), 0);
    EXPECT_EQ(book.getBestBid(), 0);
}