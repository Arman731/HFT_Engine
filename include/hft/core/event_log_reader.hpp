
#pragma once

#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>

#include "hft/core/event.hpp"

namespace hft {

class EventLogReader {
private:
    std::ifstream logFile;

    EventType stringToEventType(const std::string& type) const {
        if (type == "ORDER_ACCEPTED")
            return EventType::ORDER_ACCEPTED;

        if (type == "ORDER_REJECTED")
            return EventType::ORDER_REJECTED;

        if (type == "ORDER_CANCELLED")
            return EventType::ORDER_CANCELLED;

        if (type == "TRADE_EXECUTED")
            return EventType::TRADE_EXECUTED;

        if (type == "EXECUTION_REPORT")
            return EventType::EXECUTION_REPORT;

        if (type == "MARKET_DATA_UPDATE")
            return EventType::MARKET_DATA_UPDATE;

        throw std::runtime_error("Unknown event type in log");
    }

public:
    explicit EventLogReader(const std::string& filename)
        : logFile(filename) {

        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open event log file");
        }

        std::string header;

        if (!std::getline(logFile, header) ||
            header != "event_type,timestamp,order_id") {
            throw std::runtime_error("Invalid event log header");
        }
    }

    bool hasNext() {
        return logFile.peek() != std::ifstream::traits_type::eof();
    }

    Event readNextEvent() {
        std::string line;

        if (!std::getline(logFile, line)) {
            throw std::runtime_error("No more events to read");
        }

        std::stringstream stream(line);

        std::string type;
        std::string timestamp;
        std::string orderId;
        std::string extra;

        if (!std::getline(stream, type, ',') ||
            !std::getline(stream, timestamp, ',') ||
            !std::getline(stream, orderId, ',') ||
            std::getline(stream, extra, ',')) {
            throw std::runtime_error("Invalid event log record");
        }

        try {
            std::size_t timestampPos = 0;
            std::size_t orderIdPos = 0;

            Timestamp parsedTimestamp =
                std::stoull(timestamp, &timestampPos);

            OrderId parsedOrderId =
                std::stoull(orderId, &orderIdPos);

            if (timestampPos != timestamp.size() ||
                orderIdPos != orderId.size()) {
                throw std::runtime_error("Invalid numeric field");
            }

            return Event{
                stringToEventType(type),
                parsedTimestamp,
                parsedOrderId
            };
        }
        catch (const std::exception&) {
            throw std::runtime_error("Invalid event log record");
        }
    }
};

}
