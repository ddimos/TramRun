#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "tram_run/State.hpp"

namespace tr::app
{
    struct Event
    {
        enum class Type : uint8_t
        {
            ButtonPress,
            ButtonLongPress
        };
        Type type = Type::ButtonPress;
    };

    class App final
    {
    public:

        App();
        ~App();

        void start();

    private:
        static void mainTask(void* _pvParameter);

        state::Id getStateSafe() const;

        void transit(state::Transit _transit);
        void transitInitState(state::Transit _transit);
        void transitRunState(state::Transit _transit);

        void update();
        state::Status updateInitState();
        state::Status updateRunState();

        void onButtonPress();
        void onButtonLongPress();

        state::Id m_state = state::Id::Init;
        QueueHandle_t m_queue = nullptr;
    };

} // namespace tr::app
