
#pragma once

#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

#include "hft/core/order.hpp"
#include "hft/core/trade.hpp"
#include "hft/core/order_result.hpp"
#include "hft/core/order_lifecycle.hpp"
#include "hft/execution/simulator/order_book.hpp"

#include "hft/core/execution_report.hpp"

#include "hft/core/clock.hpp"
#include "hft/core/system_clock.hpp"

namespace hft {

class MatchingEngine {

private:

    OrderBook& orderBook;

    SystemClock defaultClock;
    Clock& clock;

    std::vector<Trade> trades;

    std::unordered_set<OrderId> usedOrderIds;

    std::unordered_map<OrderId, OrderLifecycle> orderLifecycles;
    
    std::vector<ExecutionReport> executionReports;

    void recordExecutionReport(
        OrderId orderId,
        ExecutionReportType type,
        Quantity lastExecutedQuantity,
        Price executionPrice,
        Timestamp timestamp
    ) {
        auto it = orderLifecycles.find(orderId);

        if (it == orderLifecycles.end()) {
            return;
        }

        const OrderLifecycle& lifecycle = it->second;

        executionReports.push_back({
            orderId,
            type,
            lastExecutedQuantity,
            lifecycle.filledQuantity,
            lifecycle.remainingQuantity,
            executionPrice,
            timestamp
        });
    }

    void recordRejectedReport(const Order& order) {
        executionReports.push_back({
            order.id,
            ExecutionReportType::REJECTED,
            0,
            0,
            order.quantity,
            0,
            clock.now()
        });
    }

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

    void initializeLifecycle(const Order& order) {

        orderLifecycles.emplace(
            order.id,
            OrderLifecycle{
                order.id,
                order.quantity,
                0,
                order.quantity,
                OrderLifecycleStatus::ACCEPTED
            }
        );
    }

    void recordFill(
        OrderId orderId,
        Quantity quantity,
        Price executionPrice,
        Timestamp timestamp
    ) {
        auto it = orderLifecycles.find(orderId);

        if (it == orderLifecycles.end()) {
            return;
        }

        OrderLifecycle& lifecycle = it->second;

        if (quantity > lifecycle.remainingQuantity) {
            return;
        }

        lifecycle.filledQuantity += quantity;
        lifecycle.remainingQuantity -= quantity;

        if (lifecycle.remainingQuantity == 0) {
            lifecycle.status = OrderLifecycleStatus::FILLED;

            recordExecutionReport(
                orderId,
                ExecutionReportType::FILLED,
                quantity,
                executionPrice,
                timestamp
            );
        }
        else {
            lifecycle.status = OrderLifecycleStatus::PARTIALLY_FILLED;

            recordExecutionReport(
                orderId,
                ExecutionReportType::PARTIALLY_FILLED,
                quantity,
                executionPrice,
                timestamp
            );
        }
    }

    void updateIncomingOrderStatus(OrderId orderId) {

        auto it = orderLifecycles.find(orderId);

        if (it == orderLifecycles.end()) {
            return;
        }

        OrderLifecycle& lifecycle = it->second;

        if (lifecycle.remainingQuantity == 0) {
            lifecycle.status = OrderLifecycleStatus::FILLED;
        }
        else if (lifecycle.filledQuantity > 0) {
            lifecycle.status = OrderLifecycleStatus::PARTIALLY_FILLED;
        }
        else {
            lifecycle.status = OrderLifecycleStatus::ACCEPTED;
        }
    }

    void rejectOrder(const Order& order) {

        if (order.id == 0) {
            return;
        }

        if (orderLifecycles.find(order.id) != orderLifecycles.end()) {
            return;
        }

        orderLifecycles.emplace(
            order.id,
            OrderLifecycle{
                order.id,
                order.quantity,
                0,
                order.quantity,
                OrderLifecycleStatus::REJECTED
            }
        );
    }

public:

    // Uses real system time by default.
    explicit MatchingEngine(OrderBook& book)
        : orderBook(book),
        clock(defaultClock) {
    }

    // Allows an external clock to be injected.
    MatchingEngine(OrderBook& book, Clock& suppliedClock)
        : orderBook(book),
        clock(suppliedClock) {
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

            Timestamp eventTimestamp = clock.now();

            OrderId restingOrderId = bestAsk->id;

            recordTrade(
                incomingOrder.id,
                restingOrderId,
                executionPrice,
                tradedQuantity,
                eventTimestamp
            );

            incomingOrder.quantity -= tradedQuantity;
            bestAsk->quantity -= tradedQuantity;

            recordFill(
                incomingOrder.id,
                tradedQuantity,
                executionPrice,
                eventTimestamp
            );

            recordFill(
                restingOrderId,
                tradedQuantity,
                executionPrice,
                eventTimestamp
            );

            if (bestAsk->quantity == 0) {
                orderBook.cancelOrder(restingOrderId);
            }
        }

        updateIncomingOrderStatus(incomingOrder.id);

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

            recordFill(
                incomingOrder.id,
                tradedQuantity,
                executionPrice,
                incomingOrder.timestamp
            );

            recordFill(
                restingOrderId,
                tradedQuantity,
                executionPrice,
                incomingOrder.timestamp
            );

            if (bestBid->quantity == 0) {
                orderBook.cancelOrder(restingOrderId);
            }
        }

        updateIncomingOrderStatus(incomingOrder.id);

        if (incomingOrder.quantity > 0) {
            orderBook.addOrder(incomingOrder);
        }
    }

    OrderResult processOrder(const Order& order) {

        OrderStatus status = validateOrder(order);

        if (status != OrderStatus::ACCEPTED) {
            rejectOrder(order);
            recordRejectedReport(order);
            return {status};
        }

        if (usedOrderIds.find(order.id) != usedOrderIds.end()) {
            recordRejectedReport(order);
            return {OrderStatus::DUPLICATE_ORDER_ID};
        }

        usedOrderIds.insert(order.id);

        initializeLifecycle(order);

        recordExecutionReport(
            order.id,
            ExecutionReportType::ACCEPTED,
            0,
            0,
            clock.now()
        );

        if (order.side == Side::BUY) {
            processBuyOrder(order);
        }
        else {
            processSellOrder(order);
        }

        return {OrderStatus::ACCEPTED};
    }

    bool cancelOrder(OrderId orderId) {

        auto it = orderLifecycles.find(orderId);

        if (it == orderLifecycles.end()) {
            return false;
        }

        OrderLifecycle& lifecycle = it->second;

        if (
            lifecycle.status == OrderLifecycleStatus::FILLED ||
            lifecycle.status == OrderLifecycleStatus::CANCELLED ||
            lifecycle.status == OrderLifecycleStatus::REJECTED
        ) {
            return false;
        }

        if (!orderBook.cancelOrder(orderId)) {
            return false;
        }

        lifecycle.status = OrderLifecycleStatus::CANCELLED;

        recordExecutionReport(
            orderId,
            ExecutionReportType::CANCELLED,
            0,
            0,
            clock.now()
        );

        return true;
    }

    const OrderLifecycle* getOrderLifecycle(OrderId orderId) const {

        auto it = orderLifecycles.find(orderId);

        if (it == orderLifecycles.end()) {
            return nullptr;
        }

        return &it->second;
    }

    const std::vector<ExecutionReport>& getExecutionReports() const {
        return executionReports;
    }

    const std::vector<Trade>& getTrades() const {
        return trades;
    }
};

}
