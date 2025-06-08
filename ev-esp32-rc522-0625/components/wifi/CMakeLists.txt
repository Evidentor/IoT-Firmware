idf_component_register(
        SRCS "wifi.c"
        INCLUDE_DIRS "include"
        REQUIRES wifi_provisioning esp_wifi nvs_flash
)
