#include "tram_run/Display.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

extern "C"
{
#include "ssd1306.h"
}

namespace
{
    static const char* TAG = "TR_DISPLAY";
    
    class Display final
    {
    public:
        Display();
        ~Display();

        void drawText(const char* _text, int _length, int _pos);
        void clear();
    
    private:
        SSD1306_t m_display;
    };

    Display::Display()
    {
        i2c_master_init(&m_display, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

        ESP_LOGI(TAG, "Init display");
        ssd1306_init(&m_display, 128, 64);

        ssd1306_clear_screen(&m_display, false);
        ssd1306_contrast(&m_display, 0xff);
    }

    Display::~Display()
    {
    }

    void Display::drawText(const char* _text, int _length, int _pos)
    {
        ssd1306_display_text(&m_display, _pos, _text, _length, false);
    }

    void Display::clear()
    {
        ssd1306_clear_screen(&m_display, false);
    }

    static QueueHandle_t g_queue = nullptr;
    static TaskHandle_t g_task = nullptr;

    void task(void* _pvParameter)
    {
        Display display;

        tr::display::Event event;
        while (true)
        {
            if (xQueueReceive(g_queue, &event, portMAX_DELAY))
            {
                switch (event.type)
                {
                case tr::display::Event::Type::Clear:
                    ESP_LOGI(TAG, "Clear");
                    display.clear();
                    break;
                case tr::display::Event::Type::Draw:
                    ESP_LOGI(TAG, "Text");
                    display.drawText(event.text, event.length, event.pos);
                    break;
                case tr::display::Event::Type::DrawAndClear:
                    display.clear();
                    display.drawText(event.text, event.length, event.pos);
                    break;
                }
            }
        }
    }
} // namespace

namespace tr::display
{
    void init()
    {
        ESP_LOGI(TAG, "Init");
        // TODO assert
        if (g_queue != nullptr)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        g_queue = xQueueCreate(3, sizeof(Event));

        if (g_queue == nullptr)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        // TODO the stack size is higher that it could be because the initialization needs more memory
        BaseType_t ret = xTaskCreate(task, "DisplayTask", 4096, NULL, 8, &g_task);
        if (ret != pdPASS)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }
    }

    void deinit()
    {
        if (g_queue == nullptr)
        {
            ESP_ERROR_CHECK(ESP_FAIL);
        }

        vQueueDelete(g_queue);
        vTaskDelete(g_task);
    }

    void sendEvent(const Event& _event)
    {
        xQueueSend(g_queue, &_event, portMAX_DELAY);
    }

} // namespace tr::display
