
#include <iostream>

#include "core/order.hpp"
#include "matching/matching_engine.hpp"

const char* statusToString(hft::OrderStatus status) {

    switch (status) {

        case hft::OrderStatus::ACCEPTED:
            return "ACCEPTED";

        case hft::OrderStatus::INVALID_ORDER_ID:
            return "INVALID ORDER ID";

        case hft::OrderStatus::INVALID_PRICE:
            return "INVALID PRICE";

        case hft::OrderStatus::INVALID_QUANTITY:
            return "INVALID QUANTITY";

        case hft::OrderStatus::DUPLICATE_ORDER_ID:
            return "DUPLICATE ORDER ID";

        case hft::OrderStatus::INVALID_SIDE:
            return "INVALID SIDE";
    }

    return "UNKNOWN";
}

void submitOrder(
    hft::MatchingEngine& engine,
    const hft::Order& order
) {
    hft::OrderResult result = engine.processOrder(order);

    std::cout << "Order " << order.id << ": "
              << statusToString(result.status) << '\n';
}

int main() {

    hft::OrderBook book;
    hft::MatchingEngine engine(book);

    hft::Order order1{
        1001, hft::Side::BUY, 15030, 10, 1000000
    };

    hft::Order order2{
        1001, hft::Side::SELL, 15040, 5, 1000001
    };

    hft::Order order3{
        1002, hft::Side::SELL, 15040, 5, 1000002
    };

    hft::Order invalidSide{
        1003,
        static_cast<hft::Side>(99),
        15040,
        5,
        1000003
    };

    submitOrder(engine, order1);
    submitOrder(engine, order2);
    submitOrder(engine, order3);
    submitOrder(engine, invalidSide);

    std::cout << "\n========== FINAL ORDER BOOK ==========\n";

    book.printBook();

    return 0;
}
