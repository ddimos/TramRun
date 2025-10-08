#pragma once
#include <stdint.h>

namespace tr::display
{
    constexpr unsigned MaxTextLength = 16;
    struct Event
    {
        enum class Type : uint8_t
        {
            Clear,
            Draw,
            DrawAndClear
        };
        const char* text;
        uint8_t pos = 0;
        uint8_t length = 0;
        Type type = Type::Clear;
    };

    void init();
    void deinit();
    void sendEvent(const Event& _event);

} // namespace tr::display
