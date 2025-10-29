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
            ButtonLongPress,
            Tick,
            WifiFail,
            WifiReady,
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
        void transitConnectingToWifi(state::Transit _transit);
        void transitRunState(state::Transit _transit);

        void dispatchAndTransit(const Event& _event);
        state::Status dispatchInitState(const Event& _event);
        state::Status dispatchConnectingToWifi(const Event& _event);
        state::Status dispatchRunState(const Event& _event);

        void onButtonPress();
        void onButtonLongPress();
        void onWifiReady();
        void onWifiFail();

        state::Id m_state = state::Id::Init;
        QueueHandle_t m_queue = nullptr;
    };

} // namespace tr::app
