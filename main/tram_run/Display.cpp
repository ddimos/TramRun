#include "tram_run/Display.hpp"

#include "esp_log.h"

extern "C"
{
#include "ssd1306.h"
}

namespace
{
    static SSD1306_t g_display;
    static const char* TAG = "TR_DISPLAY";
} // namespace


namespace tr
{
    Display::Display() = default;
    Display::~Display() = default;

    void Display::init()
    {
        i2c_master_init(&g_display, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

        ESP_LOGI(TAG, "Init display");
        ssd1306_init(&g_display, 128, 64);

        ssd1306_clear_screen(&g_display, false);
        ssd1306_contrast(&g_display, 0xff);
    }

    void Display::drawText(const char* _text, int _length, int _pos)
    {
        ssd1306_display_text(&g_display, _pos, _text, _length, false);
    }

    void Display::clear()
    {
        ssd1306_clear_screen(&g_display, false);
    }
} // namespace tr
