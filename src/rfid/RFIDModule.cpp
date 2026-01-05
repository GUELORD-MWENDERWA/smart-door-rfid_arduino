#include "RFIDModule.h"
#include "config.h"


RFIDModule::RFIDModule(uint8_t ssPin, uint8_t rstPin)
    : mfrc522(ssPin, rstPin),
      cardAvailable(false),
      cardPreviouslyPresent(false)
{
}

void RFIDModule::begin() {
    DEBUG_PRINTLN(F("[RFID] Initialisation SPI + MFRC522"));
    SPI.begin();
    mfrc522.PCD_Init();
    DEBUG_PRINTLN(F("[RFID] Module prêt"));
}

bool RFIDModule::poll() {
    // Vérifie s'il y a une carte présente
    bool newCardPresent = mfrc522.PICC_IsNewCardPresent();

    // Si aucune carte, réinitialiser le flag précédent
    if (!newCardPresent) {
        cardPreviouslyPresent = false;
        cardAvailable = false;
        return false;
    }

    // Si la carte est encore présente mais qu'on n'a pas attendu le retrait, ne pas relire
    if (cardPreviouslyPresent) {
        cardAvailable = false;
        return false;
    }

    // Lire la carte
    if (!mfrc522.PICC_ReadCardSerial()) {
        DEBUG_PRINTLN(F("[RFID] Erreur lecture UID"));
        return false;
    }

    // Copier UID
    for (uint8_t i = 0; i < UID_SIZE; i++) {
        if (i < mfrc522.uid.size) {
            uid[i] = mfrc522.uid.uidByte[i];
        } else {
            uid[i] = 0x00;
        }
    }

    DEBUG_PRINT(F("[RFID] Carte détectée UID: "));
    printUID(uid);

    // Marquer carte disponible et mémoire de présence
    cardAvailable = true;
    cardPreviouslyPresent = true;

    return true;
}

bool RFIDModule::hasNewCard() const {
    return cardAvailable;
}

void RFIDModule::getUID(uint8_t *buffer) {
    if (!buffer) return;

    memcpy(buffer, uid, UID_SIZE);
    DEBUG_PRINT(F("[RFID] UID fourni: "));
    printUID(uid);
}

void RFIDModule::halt() {
    DEBUG_PRINTLN(F("[RFID] Halt carte"));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

void RFIDModule::printUID(const uint8_t *uid) {
    for (uint8_t i = 0; i < UID_SIZE; i++) {
        if (uid[i] < 0x10) DEBUG_PRINT('0');
        DEBUG_PRINT(uid[i], HEX);
        DEBUG_PRINT(' ');
    }
    DEBUG_PRINTLN();
}
