#pragma once

#include "freertos/FreeRTOS.h"

void v_task_delay_ms(unsigned short ms);

TickType_t get_tick_count_from_ms(unsigned short ms);
