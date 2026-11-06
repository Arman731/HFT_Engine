#pragma once

#include <cstdint>

namespace hft {

    using OrderId = std::uint64_t;
    using Price = std::int64_t;
    using Quantity = std::uint64_t;
    using Timestamp = std::uint64_t;

    enum class Side {
        BUY,
        SELL
    };

}