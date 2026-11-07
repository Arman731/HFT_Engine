#pragma once

#include "hft/core/types.hpp"

namespace hft {

struct Trade {
    OrderId buyOrderId;
    OrderId sellOrderId;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
};

}
