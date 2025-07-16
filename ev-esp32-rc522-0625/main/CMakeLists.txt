idf_component_register(
        SRCS "ev-esp32-rc522-0625.c"
        INCLUDE_DIRS "."
        REQUIRES wifi rfid-adapter mqtt-client ev_buzzer json
)
