#include <stdio.h>
#include "esp_log.h"
#include "rfid-adapter.h"
#include "Arduino.h"
#include "wifi.h"

static const char *TAG = "ev-esp32-rc522-0625";

void rfid_card_handler(byte* cardId) {
    printf("Card UID: ");
    for (int i = 0; i < 4; i++) {
        printf("%02X ", cardId[i]);
    }
    printf("\n");
}

void app_main(void)
{
    // Setup
    init_wifi();
    initArduino();
    init_rfid();

    bool connected = is_connected();
    ESP_LOGI(TAG, "WiFi connected: %s", connected ? "true" : "false");

    // Loop
    while (true) {
        check_for_new_card(rfid_card_handler);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
