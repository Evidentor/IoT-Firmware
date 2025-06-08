#include <stdio.h>
#include "esp_log.h"
#include "wifi.h"
#include "rfid-adapter.h"
#include "Arduino.h"
#include "mqtt-client.h"

static const char *TAG = "ev-esp32-rc522-0625";

void bytes_to_hex_string(const byte *bytes, char *hex_str) {
    sprintf(hex_str, "%02X%02X%02X%02X", bytes[0], bytes[1], bytes[2], bytes[3]);
}

void rfid_card_handler(const byte* cardId) {
    char hex_str[4];
    bytes_to_hex_string(cardId, hex_str);
    ESP_LOGI(TAG, "Card ID: %s", hex_str);
}

void gpio_pins_setup(void) {
    // TODO: Hardcoded pins for now, make this configurable
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_0, 1);

    gpio_set_pull_mode(GPIO_NUM_19, GPIO_PULLUP_ONLY);  // MISO
}

void app_main(void)
{
    // Setup GPIO pins
    gpio_pins_setup();

    // Setup Wi-Fi - blocking until connected to Wi-Fi
    init_wifi();

    // Setup Arduino and rfid lib
    initArduino();
    init_rfid();

    // Setup MQTT client
    mqtt_app_start();

    ESP_LOGI(TAG, "Setup completed");

    // Loop
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (! is_wifi_connected()) {
            continue;
        };

        check_for_new_card(rfid_card_handler);
    }
}
