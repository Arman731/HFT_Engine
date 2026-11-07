
#pragma once

#include "hft/core/clock.hpp"

namespace hft {

class SimulatedClock : public Clock {
private:
    Timestamp currentTime;

public:
    explicit SimulatedClock(Timestamp startTime = 0)
        : currentTime(startTime) {
    }

    Timestamp now() const override {
        return currentTime;
    }

    void setTime(Timestamp newTime) {
        currentTime = newTime;
    }

    void advance(Timestamp delta) {
        currentTime += delta;
    }
};

}
