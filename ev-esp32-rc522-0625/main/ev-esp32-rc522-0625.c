#include <stdio.h>
#include "esp_log.h"
#include "wifi.h"
#include "rfid-adapter.h"
#include "Arduino.h"
#include "ev_buzzer.h"
#include "mqtt-client.h"
#include "cJSON.h"

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

char* create_json_message(const char* cardId) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "cardId", cardId);
    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_string;  // Remember to free this after sending
}

bool parse_access_granted(const char* json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {
        ESP_LOGW(TAG, "cJSON_Parse failed");
        return false;
    }

    cJSON *access_granted = cJSON_GetObjectItem(root, "accessGranted");
    if (!cJSON_IsBool(access_granted)) {
        ESP_LOGW(TAG, "Attribute 'accessGranted' is not a boolean");
        cJSON_Delete(root);
        return false;
    }

    bool result = cJSON_IsTrue(access_granted);
    cJSON_Delete(root);
    return result;
}

void mqtt_data_handler(const char *topic, const char *data) {
    ESP_LOGI(TAG, "Message arrived on topic: %s", topic);

    bool access_granted = parse_access_granted(data);
    if (access_granted) {
        ESP_LOGI(TAG, "Access granted");
        play_access_granted();
    } else {
        ESP_LOGI(TAG, "Access denied");
        play_access_denied();
    }
}

void rfid_card_handler(const byte* cardId) {
    // Format to hex
    char hex_str[4];
    bytes_to_hex_string(cardId, hex_str);
    ESP_LOGI(TAG, "Card ID: %s", hex_str);

    // Send to MQTT
    const char* data = create_json_message(hex_str);
    mqtt_publish_telemetry(data);
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
    mqtt_app_start(mqtt_data_handler);

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
