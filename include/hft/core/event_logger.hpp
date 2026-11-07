
#pragma once

#include <fstream>
#include <string>
#include <stdexcept>

#include "hft/core/event.hpp"

#include <stdexcept>

namespace hft {

class EventLogger {
private:
    std::ofstream logFile;

    std::string eventTypeToString(EventType type) const {
        switch (type) {
            case EventType::ORDER_ACCEPTED:
                return "ORDER_ACCEPTED";

            case EventType::ORDER_REJECTED:
                return "ORDER_REJECTED";

            case EventType::ORDER_CANCELLED:
                return "ORDER_CANCELLED";

            case EventType::TRADE_EXECUTED:
                return "TRADE_EXECUTED";

            case EventType::EXECUTION_REPORT:
                return "EXECUTION_REPORT";

            case EventType::MARKET_DATA_UPDATE:
                return "MARKET_DATA_UPDATE";
        }

        throw std::invalid_argument("Unknown event type");
    }

    std::string serializeEvent(const Event& event) const {
        return eventTypeToString(event.type) + "," +
            std::to_string(event.timestamp) + "," +
            std::to_string(event.orderId);
    }

public:
    explicit EventLogger(const std::string& filename)
        : logFile(filename, std::ios::app) {

        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open event log file");
        }

        logFile.seekp(0, std::ios::end);

        if (logFile.tellp() == std::streampos(0)) {
            logFile << "event_type,timestamp,order_id\n";
            logFile.flush();
        }
    }

    void logEvent(const Event& event) {
        logFile << serializeEvent(event) << '\n';
        logFile.flush();

        if (!logFile) {
            throw std::runtime_error("Failed to write event to log file");
        }
    }

    bool isOpen() const {
        return logFile.is_open();
    }
};

}
