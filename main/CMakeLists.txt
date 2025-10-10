idf_component_register(
    SRCS
    "tram_run/App.cpp"
    "tram_run/Display.cpp"
    "tram_run/Input.cpp"
    "tram_run/Servo.cpp"
    "main.cpp"
    INCLUDE_DIRS ".")
# TODO_1 use modular PRIV_REQUIRES (esp_driver_mcpwm)