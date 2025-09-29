
#include <stdio.h>

#include "tram_run/App.hpp"
#include "esp_log.h"

namespace
{
    static const char* TAG = "TR_MAIN";
}

tr::App g_app;

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Start TramRun!");

    g_app.start();
}
