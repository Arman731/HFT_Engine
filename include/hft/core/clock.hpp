
#pragma once

#include "hft/core/types.hpp"

namespace hft {

class Clock {
public:
    virtual ~Clock() = default;

    virtual Timestamp now() const = 0;
};

}
