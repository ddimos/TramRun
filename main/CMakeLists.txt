idf_component_register(
    SRCS
    "tram_run/App.cpp"
    "tram_run/Display.cpp"
    "tram_run/Input.cpp"
    "tram_run/Servo.cpp"
    "tram_run/Wifi.cpp"
    "main.cpp"
    PRIV_REQUIRES esp_wifi nvs_flash driver # TODO Remove driver when the display lib is fixed
    INCLUDE_DIRS ".")
# TODO_1 use modular PRIV_REQUIRES (esp_driver_mcpwm)