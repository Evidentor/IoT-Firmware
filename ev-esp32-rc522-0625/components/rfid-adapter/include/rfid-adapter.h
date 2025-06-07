#pragma once
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_rfid(void);

void check_for_new_card(void (*callback)(byte* cardId));

#ifdef __cplusplus
}
#endif
