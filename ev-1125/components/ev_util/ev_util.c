#include <stdio.h>
#include "ev_util.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TickType_t get_tick_count_from_ms(const unsigned short ms) {
    return ms / portTICK_PERIOD_MS;
}

void v_task_delay_ms(const unsigned short ms) {
    vTaskDelay(get_tick_count_from_ms(ms));
}
