#ifndef EEPROM_STORE_H
#define EEPROM_STORE_H

#include <Arduino.h>
#include <EEPROM.h>
#include "../config.h"

/*
  EEPROMStore
  - Provides safe read/write for admin PIN and badge list.
  - Simple layout with magic/version, admin PIN, badge count, badges, crc16.
  - UID_SIZE matches RFIDModule::UID_SIZE (5).
  - API kept compatible with existing main.cpp usages:
      begin()
      readAdminPIN() -> String
      writeAdminPIN(const String&)
      addBadge(const uint8_t *uid)
      removeBadge(const uint8_t *uid)
      badgeExists(const uint8_t *uid)
      getBadgeCount()
      reset()
*/

class EEPROMStore {
public:
    static const uint8_t UID_SIZE = 5;
    static const uint16_t MAX_BADGES = 50; // safe default; adapt to EEPROM size

    EEPROMStore();

    void begin();

    String readAdminPIN();
    bool writeAdminPIN(const String &pin);

    bool addBadge(const uint8_t *uid);
    bool removeBadge(const uint8_t *uid);
    bool badgeExists(const uint8_t *uid);

    uint16_t getBadgeCount();

    void reset();

private:
    // Layout offsets (bytes)
    static const uint16_t OFF_MAGIC = 0;         // uint16_t
    static const uint16_t OFF_VERSION = 2;       // uint16_t
    static const uint16_t OFF_ADMINPIN = 4;      // fixed 8 bytes (null-terminated)
    static const uint16_t OFF_BADGE_COUNT = 12;  // uint16_t
    static const uint16_t OFF_BADGES = 14;       // badges start here

    // Helper low level
    uint16_t readU16(uint16_t addr);
    void writeU16(uint16_t addr, uint16_t value);
    void writeBlock(uint16_t addr, const uint8_t *data, uint16_t len);
    void readBlock(uint16_t addr, uint8_t *data, uint16_t len);

    uint16_t computeCRC16(uint16_t uptoAddr); // compute crc over [0, uptoAddr)

    uint16_t badgeAreaSize() const;
    uint16_t eepromSize() const;
};

#endif // EEPROM_STORE_H