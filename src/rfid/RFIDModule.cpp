#include "RFIDModule.h"

RFIDModule::RFIDModule(uint8_t ssPin, uint8_t rstPin)
    : mfrc522(ssPin, rstPin),
      cardAvailable(false) {}

void RFIDModule::begin() {
    SPI.begin();
    mfrc522.PCD_Init();
    cardAvailable = false;
}

bool RFIDModule::poll() {
    cardAvailable = false;

    if (!mfrc522.PICC_IsNewCardPresent()) {
        return false;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
        return false;
    }

    uint8_t len = min((uint8_t)UID_SIZE, mfrc522.uid.size);
    for (uint8_t i = 0; i < len; i++) {
        uid[i] = mfrc522.uid.uidByte[i];
    }

    // Padding si UID < 5 octets
    for (uint8_t i = len; i < UID_SIZE; i++) {
        uid[i] = 0x00;
    }

    cardAvailable = true;
    return true;
}

bool RFIDModule::hasNewCard() const {
    return cardAvailable;
}

void RFIDModule::getUID(uint8_t *buffer) {
    for (uint8_t i = 0; i < UID_SIZE; i++) {
        buffer[i] = uid[i];
    }
}

void RFIDModule::halt() {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}
