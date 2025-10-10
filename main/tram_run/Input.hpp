#pragma once

#include "driver/gpio.h"
#include <functional>

namespace tr::input
{
    using OnButtonPressCallback = std::function<void()>;
    using OnButtonPressCallback = std::function<void()>;

    void init(gpio_num_t _gpio, OnButtonPressCallback _pressCb, OnButtonPressCallback _longPressCb);
    void deinit();

} // namespace tr::input
