#include "tram_run/Input.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

namespace
{
    static const char* TAG = "TR_INPUT";

    static gpio_num_t g_gpio = gpio_num_t::GPIO_NUM_NC;
    static tr::input::OnButtonPressCallback g_pressCb;
    static tr::input::OnButtonPressCallback g_longPressCb;

    static TaskHandle_t g_task = nullptr;

    void task(void* _pvParameter)
    {
        const TickType_t xFrequency = pdMS_TO_TICKS(200); // TODO add to config

        {
            gpio_config_t io_conf = {};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = (1ULL << g_gpio);
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

            esp_err_t config_result = gpio_config(&io_conf);
            configASSERT(config_result == ESP_OK);
        }

        TickType_t pressTime = 0;
        bool pressed = false;
        while (true)
        {
            if (gpio_get_level(g_gpio) == 0)
            {
                if (!pressed)
                {
                    pressed = true;
                    pressTime = xTaskGetTickCount();
                }
            }
            else
            {
                if (pressed)
                {
                    TickType_t releaseTime = xTaskGetTickCount();
                    if (releaseTime - pressTime >= pdMS_TO_TICKS(3000)) // TODO add to config
                        g_longPressCb();
                    else
                        g_pressCb();

                    pressed = false;
                }
            }

            vTaskDelay(xFrequency);
        }
    }

} // namespace

namespace tr::input
{
    void init(gpio_num_t _gpio, OnButtonPressCallback _pressCb, OnButtonPressCallback _longPressCb)
    {
        ESP_LOGI(TAG, "Init");

        g_gpio = _gpio;
        g_pressCb = _pressCb;
        g_longPressCb = _longPressCb;

        BaseType_t ret = xTaskCreate(task, "DisplayTask", 2048, NULL, 9, &g_task);
        configASSERT(ret == pdPASS);
    }

    void deinit()
    {
        vTaskDelete(g_task);
    }

} // namespace tr::input
