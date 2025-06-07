#include "rfid-adapter.h"
#include <SPI.h>
#include "MFRC522.h"

#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

void init_rfid() {
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("RC522 Initialized. Bring a card near...");
}

void check_for_new_card(void (*callback)(byte* cardId)) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        Serial.print("Card UID: ");
        for (byte i = 0; i < rfid.uid.size; i++) {
            Serial.print(rfid.uid.uidByte[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        rfid.PICC_HaltA();
    }
}
