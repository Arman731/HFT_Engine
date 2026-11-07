
#pragma once

#include <chrono>

#include "hft/core/clock.hpp"

namespace hft {

class SystemClock : public Clock {
public:
    Timestamp now() const override {
        auto currentTime = std::chrono::system_clock::now();

        auto nanoseconds =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                currentTime.time_since_epoch()
            ).count();

        return static_cast<Timestamp>(nanoseconds);
    }
};

}
