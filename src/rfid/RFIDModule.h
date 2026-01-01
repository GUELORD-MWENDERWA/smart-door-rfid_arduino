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
    bool poll();                  // non bloquant
    bool hasNewCard() const;
    void getUID(uint8_t *buffer); // copie UID
    void halt();

private:
    MFRC522 mfrc522;
    bool cardAvailable;
    uint8_t uid[UID_SIZE];
};

#endif
