#include <stdio.h>

#include "esp_log.h"
#include "ev_util.h"

const char *TAG = "ev-1125";

void app_main(void) {
    ESP_LOGI(TAG, "Hello from the ev-1125!");
    while (1) {
        ESP_LOGI(TAG, "Inside the for loop...");
        v_task_delay_ms(60000);
    }
}
