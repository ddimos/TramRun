#pragma once

#include <stdint.h>

namespace tr::state
{
    enum class Id : uint8_t
    {
        Init,
        ConnectingToWifi,
        Run

    };

    enum class Transit : uint8_t
    {
        Enter,
        Exit,
    };

    struct Status
    {
        enum class Type : uint8_t
        {
            Ignored,
            Handled,
            Transit
        };

        Status() = default;
        Status(Id _nextState)
            : type{Type::Transit}
            , nextState{_nextState}
        {}
        
        Type type = Type::Ignored;
        Id nextState = Id::Init;

        bool isTransitRequested() const { return type == Type::Transit; }
    };

} // namespace tr::state
