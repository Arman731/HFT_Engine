#pragma once

#include "hft/core/types.hpp"

namespace hft {

enum class EventType {
    ORDER_ACCEPTED,
    ORDER_REJECTED,
    ORDER_CANCELLED,
    TRADE_EXECUTED,
    EXECUTION_REPORT,
    MARKET_DATA_UPDATE
};

struct Event {
    EventType type;
    Timestamp timestamp;
    OrderId orderId;
};

}
