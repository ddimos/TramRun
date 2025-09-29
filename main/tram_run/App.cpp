#include "App.hpp"

#include "freertos/FreeRTOS.h"

namespace tr
{
    App::App() = default;
    App::~App() = default;

    void App::start()
    {
        m_display.init();
        
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        m_display.drawText("Start     <", 11, 3);
        
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        m_servo.init();
        int angle = -90;
        int step = 2;
        while (angle <= 90) {
        vTaskDelay(pdMS_TO_TICKS(500));
            m_servo.rotate(angle);
            char buffer[5];
            m_display.drawText(itoa(angle, buffer, 10), 5, 5);
            angle += step; 
        }
    }

    void App::mainTask(void* _pvParameter)
    {
    }

    void App::displayTask(void* _pvParameter)
    {

    }

    void App::servoTask(void* _pvParameter)
    {

    }

    App::State App::getStateSafe() const
    {
        return m_state;
    }
} // namespace tr
