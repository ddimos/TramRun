#include "App.hpp"

#include "tram_run/Display.hpp"
#include "tram_run/Servo.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"

namespace
{
    const char* INIT_TEXT = "Init";
    const char* RUN_TEXT = "Run";
} // namespace

namespace tr
{
    App::App() = default;
    App::~App() = default;

    void App::start()
    {
        // TODO make the stack size smaller by measuring the watermark

        display::init();
        servo::init();

        BaseType_t ret = xTaskCreate(&mainTask, "mainTask", 2048, this, 6, NULL);
        if (ret != pdPASS)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }
    }

    void App::mainTask(void* _pvParameter)
    {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        const TickType_t xFrequency = pdMS_TO_TICKS(1000);

        App& app = *static_cast<App*>(_pvParameter);

        app.m_state = state::Id::Init;
        app.transit(state::Transit::Enter);
    
        while (true)
        {
            app.update();
            vTaskDelayUntil( &xLastWakeTime, xFrequency );
        }
    }

    state::Id App::getStateSafe() const
    {
        return m_state;
    }

    void App::transit(state::Transit _transit)
    {
        switch (m_state)
        {
        case state::Id::Init:
            transitInitState(_transit);
            break;
        case state::Id::Run:
            transitRunState(_transit);
            break;
        }
    }

    void App::transitInitState(state::Transit _transit)
    {
        switch (_transit)
        {
        case state::Transit::Enter:
            {
                display::Event event;
                event.type = display::Event::Type::DrawAndClear;
                event.text = INIT_TEXT;
                event.length = 4;
                display::sendEvent(event);
            }
            {
                servo::Event event;
                event.desiredRotationDeg = 10;
                servo::sendEvent(event);
            }
            break;
        case state::Transit::Exit:
            break;
        }
    }

    void App::transitRunState(state::Transit _transit)
    {
        switch (_transit)
        {
        case state::Transit::Enter:
            {
                display::Event event;
                event.type = display::Event::Type::DrawAndClear;
                event.text = RUN_TEXT;
                event.length = 3;
                display::sendEvent(event);
            }
            {
                servo::Event event;
                event.desiredRotationDeg = 70;
                servo::sendEvent(event);
            }
            break;
        case state::Transit::Exit:
            break;
        }
    }

    void App::update()
    {
        state::Status status;
        switch (m_state)
        {
        case state::Id::Init:
            status = updateInitState();
            break;
        case state::Id::Run:
            status = updateRunState();
            break;
        }
        if (status.isTransitRequested())
        {
            // TODO ASSERT(m_state != status.nextState, "Cannot transit to the same state");
            transit(state::Transit::Exit);
            m_state = status.nextState;
            transit(state::Transit::Enter);
        }
    }

    state::Status App::updateInitState()
    {
        state::Status status;
        static int ii = 5;
        --ii;
        if (ii < 0)
        {
            status = state::Status(state::Id::Run);
        }
        return status;
    }

    state::Status App::updateRunState()
    {
        state::Status status;

        static int ii = 7;
        --ii;
        if (ii < 0)
        {
            status = state::Status(state::Id::Init);
        }

        return status;
    }

} // namespace tr
