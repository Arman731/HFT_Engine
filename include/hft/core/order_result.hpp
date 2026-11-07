#pragma once

namespace hft {

enum class OrderStatus {
    ACCEPTED,
    INVALID_ORDER_ID,
    INVALID_PRICE,
    INVALID_QUANTITY,
    DUPLICATE_ORDER_ID,
    INVALID_SIDE
};

struct OrderResult {
    OrderStatus status;

    bool accepted() const {
        return status == OrderStatus::ACCEPTED;
    }
};

}