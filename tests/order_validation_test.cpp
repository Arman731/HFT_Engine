#include <gtest/gtest.h>

#include "core/order.hpp"
#include "matching/matching_engine.hpp"

TEST(OrderValidation, AcceptsValidOrder) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order order{
        1001,
        hft::Side::BUY,
        15030,
        10,
        1000000
    };

    hft::OrderResult result = engine.processOrder(order);

    EXPECT_EQ(
        result.status,
        hft::OrderStatus::ACCEPTED
    );
}


TEST(OrderValidation, RejectsZeroQuantity) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order order{
        1002, hft::Side::BUY, 15030, 0, 1000000
    };

    hft::OrderResult result = engine.processOrder(order);

    EXPECT_EQ(
        result.status,
        hft::OrderStatus::INVALID_QUANTITY
    );

    EXPECT_EQ(book.getBestBid(), 0);
}


TEST(OrderValidation, RejectsNegativePrice) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order order{
        1003, hft::Side::BUY, -15030, 10, 1000000
    };

    hft::OrderResult result = engine.processOrder(order);

    EXPECT_EQ(
        result.status,
        hft::OrderStatus::INVALID_PRICE
    );

    EXPECT_EQ(book.getBestBid(), 0);
}


TEST(OrderValidation, RejectsDuplicateOrderID) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order firstOrder{
        1004, hft::Side::BUY, 15030, 10, 1000000
    };

    hft::Order duplicateOrder{
        1004, hft::Side::SELL, 15040, 5, 1000001
    };

    hft::OrderResult firstResult =
        engine.processOrder(firstOrder);

    hft::OrderResult secondResult =
        engine.processOrder(duplicateOrder);

    EXPECT_EQ(
        firstResult.status,
        hft::OrderStatus::ACCEPTED
    );

    EXPECT_EQ(
        secondResult.status,
        hft::OrderStatus::DUPLICATE_ORDER_ID
    );

    EXPECT_EQ(book.getBestBid(), 15030);
    EXPECT_EQ(book.getBestAsk(), 0);
}


TEST(OrderValidation, RejectsInvalidSide) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order order{
        1005,
        static_cast<hft::Side>(99),
        15030,
        10,
        1000000
    };

    hft::OrderResult result = engine.processOrder(order);

    EXPECT_EQ(
        result.status,
        hft::OrderStatus::INVALID_SIDE
    );

    EXPECT_EQ(book.getBestBid(), 0);
    EXPECT_EQ(book.getBestAsk(), 0);
}


TEST(MatchingEngine, BuyOrderPartiallyExecutes) {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    // Resting SELL order: 10 units
    hft::Order sellOrder{
        1001,
        hft::Side::SELL,
        15040,
        10,
        1000000
    };

    engine.processOrder(sellOrder);

    // Incoming BUY order: 15 units
    hft::Order buyOrder{
        1002,
        hft::Side::BUY,
        15040,
        15,
        1000001
    };

    engine.processOrder(buyOrder);

    const auto& trades = engine.getTrades();

    // Exactly one trade should have occurred.
    ASSERT_EQ(trades.size(), 1);

    // Verify execution details.
    EXPECT_EQ(trades[0].buyOrderId, 1002);
    EXPECT_EQ(trades[0].sellOrderId, 1001);
    EXPECT_EQ(trades[0].price, 15040);
    EXPECT_EQ(trades[0].quantity, 10);

    // SELL order should be completely filled.
    EXPECT_EQ(book.getBestAsk(), 0);

    // Remaining 5 units of BUY should rest in the book.
    EXPECT_EQ(book.getBestBid(), 15040);

    hft::Order* remainingBid = book.getBestBidOrder();

    ASSERT_NE(remainingBid, nullptr);
    EXPECT_EQ(remainingBid->id, 1002);
    EXPECT_EQ(remainingBid->quantity, 5);
}

