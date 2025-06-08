#include <stdio.h>
#include "esp_log.h"
#include "wifi.h"
#include "rfid-adapter.h"
#include "Arduino.h"
#include "mqtt-client.h"

static const char *TAG = "ev-esp32-rc522-0625";

void rfid_card_handler(const byte* cardId) {
    printf("Card UID: ");
    for (int i = 0; i < 4; i++) {
        printf("%02X ", cardId[i]);
    }
    printf("\n");
}

void app_main(void)
{
    // Setup Wi-Fi - blocking until connected to Wi-Fi
    init_wifi();

    // Setup Arduino and rfid lib
    initArduino();
    init_rfid();

    // Setup MQTT client
    mqtt_app_start();

    // Loop
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (! is_wifi_connected()) {
            continue;
        };

        check_for_new_card(rfid_card_handler);
    }
}
