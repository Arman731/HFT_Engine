
#pragma once

#include "hft/core/types.hpp"

namespace hft {

enum class ExecutionReportType {
    ACCEPTED,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

struct ExecutionReport {
    OrderId orderId;

    ExecutionReportType type;

    Quantity lastExecutedQuantity;
    Quantity cumulativeFilledQuantity;
    Quantity remainingQuantity;

    Price executionPrice;

    Timestamp timestamp;
};

}
