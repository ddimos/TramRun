
#include <stdio.h>

#include "tram_run/App.hpp"

tr::App g_app;

extern "C" void app_main(void)
{
    printf("Hello world!\n");

    g_app.start();
}
