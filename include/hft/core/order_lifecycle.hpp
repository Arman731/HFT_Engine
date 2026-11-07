
#pragma once

#include "hft/core/types.hpp"

namespace hft {

enum class OrderLifecycleStatus {
    ACCEPTED,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

struct OrderLifecycle {
    OrderId orderId;

    Quantity originalQuantity;
    Quantity filledQuantity;
    Quantity remainingQuantity;

    OrderLifecycleStatus status;
};

}
