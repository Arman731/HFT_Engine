#pragma once

#include "hft/core/types.hpp"

namespace hft {

    struct Order {

        OrderId id;

        Side side;

        Price price;

        Quantity quantity;

        Timestamp timestamp;

    };

}
