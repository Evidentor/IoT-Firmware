#include "rfid-adapter.h"
#include <SPI.h>
#include "MFRC522.h"

// TODO: Hardcoded pins for now, make this configurable
#define SS_PIN 5
#define RST_PIN 4

static const char *TAG = "rfid-adapter";

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

extern "C" void init_rfid() {
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();
    ESP_LOGI(TAG, "RC522 Initialized");
}

void reinit_rfid() {
    rfid.PCD_Init();
    ESP_LOGI(TAG, "RC522 Reinitialized");
}

extern "C" void check_for_new_card(void (*callback)(const byte* cardId)) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        callback(rfid.uid.uidByte);
        rfid.PICC_HaltA();
    }
}
