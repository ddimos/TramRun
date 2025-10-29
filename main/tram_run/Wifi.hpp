#pragma once

#include <functional>

namespace tr::wifi
{
    enum class State
    {
        Ready,
        NotAbleToConnect
    };
    using OnWifiStateCallback = std::function<void(State)>;

    void init(OnWifiStateCallback _callback);
    void deinit();
    void start();
    void stop();

} // namespace tr::wifi
