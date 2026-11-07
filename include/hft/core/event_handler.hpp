#pragma once

#include "hft/core/event.hpp"

namespace hft {

class EventHandler {
public:
    virtual ~EventHandler() = default;

    virtual void onEvent(const Event& event) = 0;
};

}
