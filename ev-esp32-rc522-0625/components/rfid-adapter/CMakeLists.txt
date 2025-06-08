idf_component_register(SRCS "rfid-adapter.cpp"
        INCLUDE_DIRS "include"
#        REQUIRES rfid
        REQUIRES arduino-esp32 rfid-lib-precompiled
)
