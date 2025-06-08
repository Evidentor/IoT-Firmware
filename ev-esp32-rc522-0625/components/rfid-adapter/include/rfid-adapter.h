#pragma once
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_rfid(void);

void reinit_rfid(void);

void check_for_new_card(void (*callback)(const byte* cardId));

#ifdef __cplusplus
}
#endif
