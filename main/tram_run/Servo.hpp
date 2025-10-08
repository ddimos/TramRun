#pragma once
#include <stdint.h>

namespace tr::servo
{
    struct Event
    {
        uint8_t desiredRotationDeg = 0;
    };

    void init();
    void deinit();
    void sendEvent(Event _event);

} // namespace tr::servo
