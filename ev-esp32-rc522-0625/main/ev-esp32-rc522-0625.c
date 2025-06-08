#include <stdio.h>
#include "esp_log.h"
#include "wifi.h"
#include "rfid-adapter.h"
#include "Arduino.h"
#include "ev_buzzer.h"
#include "mqtt-client.h"

#define LOOP_TASK_DELAY_MS 1000
#define BEFORE_INIT_RFID_DELAY_MS 500
#define AFTER_BUZZER_PLAY_RFID_REINIT_DELAY_MS 100

static const char *TAG = "ev-esp32-rc522-0625";

void play_access_granted(void) {
    buzzer_on();
    vTaskDelay(pdMS_TO_TICKS(1500));
    buzzer_off();

    // TODO: Temporary patch which fixes the rfid power cutoff after buzzer beep
    vTaskDelay(AFTER_BUZZER_PLAY_RFID_REINIT_DELAY_MS);
    reinit_rfid();
}

void play_access_denied(void) {
    for (int i = 0; i < 3; i++) {
        buzzer_on();
        vTaskDelay(pdMS_TO_TICKS(200));
        buzzer_off();
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // TODO: Temporary patch which fixes the rfid power cutoff after buzzer beep
    vTaskDelay(AFTER_BUZZER_PLAY_RFID_REINIT_DELAY_MS);
    reinit_rfid();
}

void bytes_to_hex_string(const byte *bytes, char *hex_str) {
    sprintf(hex_str, "%02X%02X%02X%02X", bytes[0], bytes[1], bytes[2], bytes[3]);
}

void rfid_card_handler(const byte* cardId) {
    char hex_str[4];
    bytes_to_hex_string(cardId, hex_str);
    ESP_LOGI(TAG, "Card ID: %s", hex_str);
    play_access_granted();
}

void gpio_pins_setup(void) {
    // TODO: Hardcoded pins for now, make this configurable
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_0, 1);

    gpio_set_pull_mode(GPIO_NUM_19, GPIO_PULLUP_ONLY);  // MISO
}

void app_main(void)
{
    // Setup Wi-Fi - blocking until connected to Wi-Fi
    init_wifi();

    // Setup MQTT client
    mqtt_app_start();

    // Init buzzer
    buzzer_init();

    // Rest for BEFORE_INIT_RFID_DELAY_MS seconds
    vTaskDelay(pdMS_TO_TICKS(BEFORE_INIT_RFID_DELAY_MS));

    // Setup GPIO pins
    gpio_pins_setup();

    // Setup Arduino and rfid lib
    initArduino();
    init_rfid();

    ESP_LOGI(TAG, "Setup completed");

    // Loop
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(LOOP_TASK_DELAY_MS));

        if (! is_wifi_connected()) {
            continue;
        }

        check_for_new_card(rfid_card_handler);
    }
}
