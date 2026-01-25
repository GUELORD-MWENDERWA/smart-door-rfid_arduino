#ifndef RFID_MODULE_H
#define RFID_MODULE_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

class RFIDModule {
public:
    static const uint8_t UID_SIZE = 5;

    RFIDModule(uint8_t ssPin, uint8_t rstPin);

    void begin();
    bool poll();                  // vérifie la présence d'une carte (non bloquant)
    bool hasNewCard() const;      // retourne true si une nouvelle carte est détectée
    void getUID(uint8_t *buffer); // copie UID dans buffer
    void halt();                  // met fin à la communication avec la carte

private:
    MFRC522 mfrc522;
    bool cardAvailable;           // flag pour indiquer qu'une carte est prête
    bool cardPreviouslyPresent;   // flag pour empêcher lecture répétée si carte non retirée
    uint8_t uid[UID_SIZE];

    void printUID(const uint8_t *uid); // debug: affiche UID sur Serial
};

#endif