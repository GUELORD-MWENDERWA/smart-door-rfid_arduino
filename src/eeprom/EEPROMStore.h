#ifndef EEPROM_STORE_H
#define EEPROM_STORE_H

#include <Arduino.h>
#include <EEPROM.h>

class EEPROMStore {
public:
    static const uint8_t UID_SIZE = 5;
    static const uint8_t MAX_BADGES = 50;

    void begin();

    bool addBadge(const uint8_t *uid);
    bool removeBadge(const uint8_t *uid);
    bool badgeExists(const uint8_t *uid);

    uint8_t getBadgeCount();

    void reset();

private:
    static const int EEPROM_MAGIC_ADDR = 0;
    static const uint8_t EEPROM_MAGIC = 0x42;

    static const int EEPROM_COUNT_ADDR = 1;
    static const int EEPROM_BASE_ADDR  = 2;

    uint8_t badgeCount;

    int badgeAddress(uint8_t index);
    bool compareUID(const uint8_t *a, const uint8_t *b);
};

#endif
