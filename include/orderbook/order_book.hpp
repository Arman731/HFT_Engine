#pragma once

#include <map>
#include <deque>

#include "core/order.hpp"

#include <unordered_map>
#include <algorithm>

#include <iostream>

namespace hft {

    class OrderBook {

    private:

        std::map<
            Price,
            std::deque<Order>,
            std::greater<Price>
        > bids;

        std::map<
            Price,
            std::deque<Order>
        > asks;

        std::unordered_map<OrderId, Price> orderLookup;

    public:


        OrderBook() = default;

        // Add a new order
        void addOrder(const Order& order) {

            if (order.side == Side::BUY) {

                bids[order.price].push_back(order);

            }
            else {

                asks[order.price].push_back(order);

            }

            orderLookup[order.id] = order.price;

        }

        bool cancelOrder(OrderId id) {

            auto it = orderLookup.find(id);

            if (it == orderLookup.end()) {
                return false;
            }

            Price price = it->second;

            // Search the bid side
            auto bidIt = bids.find(price);

            if (bidIt != bids.end()) {

                auto& orders = bidIt->second;

                auto orderIt = std::find_if(
                    orders.begin(),
                    orders.end(),
                    [id](const Order& order) {
                        return order.id == id;
                    }
                );

                if (orderIt != orders.end()) {

                    orders.erase(orderIt);

                    if (orders.empty()) {
                        bids.erase(bidIt);
                    }

                    orderLookup.erase(it);

                    return true;
                }
            }

            // Search the ask side
            auto askIt = asks.find(price);

            if (askIt != asks.end()) {

                auto& orders = askIt->second;

                auto orderIt = std::find_if(
                    orders.begin(),
                    orders.end(),
                    [id](const Order& order) {
                        return order.id == id;
                    }
                );

                if (orderIt != orders.end()) {

                    orders.erase(orderIt);

                    if (orders.empty()) {
                        asks.erase(askIt);
                    }

                    orderLookup.erase(it);

                    return true;
                }
            }

            return false;
        }

        // Get highest buy price
        Price getBestBid() const {

            if (bids.empty()) {
                return 0;
            }

            return bids.begin()->first;

        }

        // Get lowest sell price
        Price getBestAsk() const {

            if (asks.empty()) {
                return 0;
            }

            return asks.begin()->first;

        }

        Order* getBestAskOrder() {
            if (asks.empty()) {
                return nullptr;
            }

            auto& orders = asks.begin()->second;

            if (orders.empty()) {
                return nullptr;
            }

            return &orders.front();
        }

        Order* getBestBidOrder() {
            if (bids.empty()) {
                return nullptr;
            }

            auto& orders = bids.begin()->second;

            if (orders.empty()) {
                return nullptr;
            }

            return &orders.front();
        }

        void printBook() const {
            std::cout << "\n========== ORDER BOOK ==========\n";

            std::cout << "\nASKS (Sell Orders)\n";

            for (const auto& [price, orders] : asks) {
                std::cout << "Price: " << price << " | ";

                for (const auto& order : orders) {
                    std::cout << "[ID: " << order.id
                            << ", Qty: " << order.quantity << "] ";
                }

                std::cout << '\n';
            }

            std::cout << "\nBIDS (Buy Orders)\n";

            for (const auto& [price, orders] : bids) {
                std::cout << "Price: " << price << " | ";

                for (const auto& order : orders) {
                    std::cout << "[ID: " << order.id
                            << ", Qty: " << order.quantity << "] ";
                }

                std::cout << '\n';
            }

            std::cout << "================================\n";
        }

     };

}
