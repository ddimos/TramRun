#include "App.hpp"

#include "tram_run/Display.hpp"
#include "tram_run/Input.hpp"
#include "tram_run/Servo.hpp"
#include "tram_run/Wifi.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"

namespace
{
    static const char* TAG = "TR_APP";

    const char* INIT_TEXT = "Init";
    const char* WIFI_TEXT = "Wifi";
    const char* RUN_TEXT = "Run";

    constexpr gpio_num_t ButtonGpio = GPIO_NUM_19;
} // namespace

namespace tr::app
{
    App::App() = default;
    App::~App() = default;

    void App::start()
    {
        // TODO make the stack size smaller by measuring the watermark

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        wifi::init(
            [this](wifi::State _state){
                if (_state == wifi::State::Ready)
                    this->onWifiReady();
                else if (_state == wifi::State::NotAbleToConnect)
                    this->onWifiFail();
                else
                {
                    configASSERT(false);
                }
            }
        );

        display::init();
        input::init(
            ButtonGpio,
            [this](){
                this->onButtonPress();
            },
            [this](){
                this->onButtonLongPress();
            }
        );
        servo::init();

        configASSERT(m_queue == nullptr);
        m_queue = xQueueCreate(5, sizeof(Event));
        configASSERT(m_queue != nullptr);

        BaseType_t ret = xTaskCreate(&mainTask, "mainTask", 2048, this, 6, NULL);
        configASSERT(ret == pdPASS);
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
            Event event;
            while (xQueueReceive(app.m_queue, &event, 0))
            {
                ESP_LOGI(TAG, "Handling %d ", (int)event.type);
                app.dispatchAndTransit(event);
            }

            event.type = Event::Type::Tick;
            app.dispatchAndTransit(event);
            
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
            case state::Id::ConnectingToWifi:
            transitConnectingToWifi(_transit);
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

    void App::transitConnectingToWifi(state::Transit _transit)
    {
        switch (_transit)
        {
            case state::Transit::Enter:
            {
                wifi::start();

                display::Event event;
                event.type = display::Event::Type::DrawAndClear;
                event.text = WIFI_TEXT;
                event.length = 4;
                display::sendEvent(event);
                break;
            }
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

    void App::dispatchAndTransit(const Event& _event)
    {
        state::Status status;
        switch (m_state)
        {
            case state::Id::Init:
                status = dispatchInitState(_event);
                break;
            case state::Id::ConnectingToWifi:
                status = dispatchConnectingToWifi(_event);
                break;
            case state::Id::Run:
                status = dispatchRunState(_event);
                break;
        }
        if (status.isTransitRequested())
        {
            ESP_LOGI(TAG, "Transiting %d ", (int)status.nextState);
            configASSERT(m_state != status.nextState);
            transit(state::Transit::Exit);
            m_state = status.nextState;
            transit(state::Transit::Enter);
        }
    }

    state::Status App::dispatchInitState(const Event& _event)
    {
        state::Status status;

        switch (_event.type)
        {
            case Event::Type::Tick:
            {
                static int ii = 5;
                --ii;
                if (ii < 0)
                {
                    ii = 5;
                    status = state::Status(state::Id::ConnectingToWifi);
                }
                break;
            }
            default:
                break;
        }
        return status;
    }

    state::Status App::dispatchConnectingToWifi(const Event& _event)
    {
        state::Status status;
        switch (_event.type)
        {
            case Event::Type::WifiReady:
            {
                status = state::Status(state::Id::Run);
                break;
            }
            case Event::Type::WifiFail:
            {
                ESP_LOGE(TAG, "Unable to connect to WIFI!");
                break;
            }
            default:
                break;
        }
        return status;
    }

    state::Status App::dispatchRunState(const Event& _event)
    {
        state::Status status;

        return status;
    }

    void App::onButtonPress()
    {
        Event event;
        event.type = Event::Type::ButtonPress;
        xQueueSend(m_queue, &event, portMAX_DELAY);
    }

    void App::onButtonLongPress()
    {
        Event event;
        event.type = Event::Type::ButtonLongPress;
        xQueueSend(m_queue, &event, portMAX_DELAY);
    }

    void App::onWifiReady()
    {
        Event event;
        event.type = Event::Type::WifiReady;
        xQueueSend(m_queue, &event, portMAX_DELAY);
    }

    void App::onWifiFail()
    {
        Event event;
        event.type = Event::Type::WifiFail;
        xQueueSend(m_queue, &event, portMAX_DELAY);
    }

} // namespace tr::app
