idf_component_register(SRCS "rfid-adapter.cpp"
        INCLUDE_DIRS "include"
        REQUIRES arduino-esp32 rfid-lib-precompiled
)
