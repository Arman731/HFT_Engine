#pragma once

#include <vector>

#include "core/order.hpp"
#include "core/trade.hpp"
#include "orderbook/order_book.hpp"

#include <algorithm>

#include <iostream>

#include <unordered_set>

#include "core/order_result.hpp"

namespace hft {

class MatchingEngine {
private:
    OrderBook& orderBook;

    std::vector<Trade> trades;

    std::unordered_set<OrderId> usedOrderIds;

    void recordTrade(
        OrderId buyId,
        OrderId sellId,
        Price price,
        Quantity quantity,
        Timestamp timestamp
    ) {
        Trade trade{
            buyId,
            sellId,
            price,
            quantity,
            timestamp
        };

        trades.push_back(trade);
    }

    OrderStatus validateOrder(const Order& order) const {

        if (order.id == 0) {
            return OrderStatus::INVALID_ORDER_ID;
        }

        if (order.price <= 0) {
            return OrderStatus::INVALID_PRICE;
        }

        if (order.quantity == 0) {
            return OrderStatus::INVALID_QUANTITY;
        }

        if (order.side != Side::BUY && order.side != Side::SELL) {
            return OrderStatus::INVALID_SIDE;
        }

        return OrderStatus::ACCEPTED;
    }

public:
    explicit MatchingEngine(OrderBook& book)
        : orderBook(book) {
    }

    void processBuyOrder(Order incomingOrder) {

        while (incomingOrder.quantity > 0) {

            Order* bestAsk = orderBook.getBestAskOrder();

            if (bestAsk == nullptr) {
                break;
            }

            if (incomingOrder.price < bestAsk->price) {
                break;
            }

            Quantity tradedQuantity = std::min(
                incomingOrder.quantity,
                bestAsk->quantity
            );

            Price executionPrice = bestAsk->price;
            OrderId restingOrderId = bestAsk->id;

            recordTrade(
                incomingOrder.id,
                restingOrderId,
                executionPrice,
                tradedQuantity,
                incomingOrder.timestamp
            );

            incomingOrder.quantity -= tradedQuantity;
            bestAsk->quantity -= tradedQuantity;

            if (bestAsk->quantity == 0) {
                orderBook.cancelOrder(restingOrderId);
            }
        }

        if (incomingOrder.quantity > 0) {
            orderBook.addOrder(incomingOrder);
        }
    }

    void processSellOrder(Order incomingOrder) {

            while (incomingOrder.quantity > 0) {

                Order* bestBid = orderBook.getBestBidOrder();

                if (bestBid == nullptr) {
                    break;
                }

                if (incomingOrder.price > bestBid->price) {
                    break;
                }

                Quantity tradedQuantity = std::min(
                    incomingOrder.quantity,
                    bestBid->quantity
                );

                Price executionPrice = bestBid->price;
                OrderId restingOrderId = bestBid->id;

                recordTrade(
                    restingOrderId,
                    incomingOrder.id,
                    executionPrice,
                    tradedQuantity,
                    incomingOrder.timestamp
                );

                incomingOrder.quantity -= tradedQuantity;
                bestBid->quantity -= tradedQuantity;

                if (bestBid->quantity == 0) {
                    orderBook.cancelOrder(restingOrderId);
                }
            }

            if (incomingOrder.quantity > 0) {
                orderBook.addOrder(incomingOrder);
            }
        }

    OrderResult processOrder(const Order& order) {

        OrderStatus status = validateOrder(order);

            if (status != OrderStatus::ACCEPTED) {
                 return {status};
            }

            if (usedOrderIds.find(order.id) != usedOrderIds.end()) {
                return {OrderStatus::DUPLICATE_ORDER_ID};
            }

            usedOrderIds.insert(order.id);

            if (order.side == Side::BUY) {
                processBuyOrder(order);
            }
            else {
                processSellOrder(order);
            }

            return {OrderStatus::ACCEPTED};
        }

    const std::vector<Trade>& getTrades() const {
            return trades;
        }
};

}