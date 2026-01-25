#include "EEPROMStore.h"

#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#endif

EEPROMStore::EEPROMStore() {}

void EEPROMStore::begin() {
    // Validate header; if invalid, initialize defaults
    uint16_t magic = readU16(OFF_MAGIC);
    uint16_t version = readU16(OFF_VERSION);

    if (magic != EEPROM_MAGIC || version != EEPROM_VERSION) {
        DEBUG_PRINTLN(F("[EEPROM] Invalid header, initializing defaults"));
        writeU16(OFF_MAGIC, EEPROM_MAGIC);
        writeU16(OFF_VERSION, EEPROM_VERSION);

        // admin PIN (8 bytes)
        char pinBuf[8];
        memset(pinBuf, 0, sizeof(pinBuf));
        strncpy(pinBuf, DEFAULT_ADMIN_PIN, sizeof(pinBuf) - 1);
        writeBlock(OFF_ADMINPIN, (const uint8_t*)pinBuf, sizeof(pinBuf));

        writeU16(OFF_BADGE_COUNT, 0);

        // zero badge area
        uint16_t badgesArea = badgeAreaSize();
        uint8_t zeros[32];
        memset(zeros, 0, sizeof(zeros));
        uint16_t remaining = badgesArea;
        uint16_t addr = OFF_BADGES;
        while (remaining) {
            uint16_t chunk = remaining > sizeof(zeros) ? sizeof(zeros) : remaining;
            writeBlock(addr, zeros, chunk);
            addr += chunk;
            remaining -= chunk;
        }

        // write CRC at end
        uint16_t crc = computeCRC16(OFF_BADGES + badgeAreaSize());
        writeU16(eepromSize() - 2, crc);
    } else {
        // verify crc
        uint16_t storedCrc = readU16(eepromSize() - 2);
        uint16_t crc = computeCRC16(OFF_BADGES + badgeAreaSize());
        if (storedCrc != crc) {
            DEBUG_PRINTLN(F("[EEPROM] CRC mismatch, performing reset"));
            reset();
        } else {
            DEBUG_PRINTLN(F("[EEPROM] Header ok"));
        }
    }
}

String EEPROMStore::readAdminPIN() {
    char pinBuf[8];
    memset(pinBuf, 0, sizeof(pinBuf));
    readBlock(OFF_ADMINPIN, (uint8_t*)pinBuf, sizeof(pinBuf));
    return String(pinBuf);
}

bool EEPROMStore::writeAdminPIN(const String &pin) {
    if (pin.length() == 0 || pin.length() >= 8) return false;
    char pinBuf[8];
    memset(pinBuf, 0, sizeof(pinBuf));
    pin.toCharArray(pinBuf, sizeof(pinBuf));
    writeBlock(OFF_ADMINPIN, (const uint8_t*)pinBuf, sizeof(pinBuf));
    // update crc
    uint16_t crc = computeCRC16(OFF_BADGES + badgeAreaSize());
    writeU16(eepromSize() - 2, crc);
    DEBUG_PRINTLN(F("[EEPROM] Admin PIN updated"));
    return true;
}

bool EEPROMStore::addBadge(const uint8_t *uid) {
    if (!uid) return false;
    uint16_t count = getBadgeCount();
    if (count >= MAX_BADGES) {
        DEBUG_PRINTLN(F("[EEPROM] Max badges reached"));
        return false;
    }
    // check exists
    if (badgeExists(uid)) return false;

    uint16_t writeAddr = OFF_BADGES + (count * UID_SIZE);
    writeBlock(writeAddr, uid, UID_SIZE);
    writeU16(OFF_BADGE_COUNT, count + 1);
    // update crc
    uint16_t crc = computeCRC16(OFF_BADGES + badgeAreaSize());
    writeU16(eepromSize() - 2, crc);
    DEBUG_PRINTLN(F("[EEPROM] Badge added"));
    return true;
}

bool EEPROMStore::removeBadge(const uint8_t *uid) {
    if (!uid) return false;
    uint16_t count = getBadgeCount();
    if (count == 0) return false;

    // find index
    int found = -1;
    for (uint16_t i = 0; i < count; i++) {
        uint8_t buf[UID_SIZE];
        readBlock(OFF_BADGES + i * UID_SIZE, buf, UID_SIZE);
        if (memcmp(buf, uid, UID_SIZE) == 0) {
            found = i;
            break;
        }
    }
    if (found == -1) return false;

    // shift badges down
    for (uint16_t i = found; i < count - 1; i++) {
        uint8_t buf[UID_SIZE];
        readBlock(OFF_BADGES + (i + 1) * UID_SIZE, buf, UID_SIZE);
        writeBlock(OFF_BADGES + i * UID_SIZE, buf, UID_SIZE);
    }
    // zero last slot
    uint8_t zero[UID_SIZE];
    memset(zero, 0, UID_SIZE);
    writeBlock(OFF_BADGES + (count - 1) * UID_SIZE, zero, UID_SIZE);

    writeU16(OFF_BADGE_COUNT, count - 1);
    // update crc
    uint16_t crc = computeCRC16(OFF_BADGES + badgeAreaSize());
    writeU16(eepromSize() - 2, crc);
    DEBUG_PRINTLN(F("[EEPROM] Badge removed"));
    return true;
}

bool EEPROMStore::badgeExists(const uint8_t *uid) {
    if (!uid) return false;
    uint16_t count = getBadgeCount();
    for (uint16_t i = 0; i < count; i++) {
        uint8_t buf[UID_SIZE];
        readBlock(OFF_BADGES + i * UID_SIZE, buf, UID_SIZE);
        if (memcmp(buf, uid, UID_SIZE) == 0) {
            return true;
        }
    }
    return false;
}

uint16_t EEPROMStore::getBadgeCount() {
    uint16_t c = readU16(OFF_BADGE_COUNT);
    if (c > MAX_BADGES) return 0; // sanity
    return c;
}

void EEPROMStore::reset() {
    // Reset header and clear badges
    writeU16(OFF_MAGIC, EEPROM_MAGIC);
    writeU16(OFF_VERSION, EEPROM_VERSION);
    char pinBuf[8];
    memset(pinBuf, 0, sizeof(pinBuf));
    strncpy(pinBuf, DEFAULT_ADMIN_PIN, sizeof(pinBuf) - 1);
    writeBlock(OFF_ADMINPIN, (const uint8_t*)pinBuf, sizeof(pinBuf));
    writeU16(OFF_BADGE_COUNT, 0);
    // zero badge area
    uint16_t badgesArea = badgeAreaSize();
    uint8_t zeros[32];
    memset(zeros, 0, sizeof(zeros));
    uint16_t remaining = badgesArea;
    uint16_t addr = OFF_BADGES;
    while (remaining) {
        uint16_t chunk = remaining > sizeof(zeros) ? sizeof(zeros) : remaining;
        writeBlock(addr, zeros, chunk);
        addr += chunk;
        remaining -= chunk;
    }
    // update crc
    uint16_t crc = computeCRC16(OFF_BADGES + badgeAreaSize());
    writeU16(eepromSize() - 2, crc);
    DEBUG_PRINTLN(F("[EEPROM] Reset complete"));
}

/* ----- low level helpers ----- */

uint16_t EEPROMStore::readU16(uint16_t addr) {
    uint8_t b0 = EEPROM.read(addr);
    uint8_t b1 = EEPROM.read(addr + 1);
    return ((uint16_t)b1 << 8) | b0;
}

void EEPROMStore::writeU16(uint16_t addr, uint16_t value) {
    EEPROM.update(addr, value & 0xFF);
    EEPROM.update(addr + 1, (value >> 8) & 0xFF);
}

void EEPROMStore::writeBlock(uint16_t addr, const uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        EEPROM.update(addr + i, data[i]);
    }
}

void EEPROMStore::readBlock(uint16_t addr, uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        data[i] = EEPROM.read(addr + i);
    }
}

// Simple CRC16-CCITT implementation over EEPROM area [0, uptoAddr)
uint16_t EEPROMStore::computeCRC16(uint16_t uptoAddr) {
    uint16_t crc = 0xFFFF;
    uint16_t maxAddr = uptoAddr;
    if (maxAddr > eepromSize() - 2) maxAddr = eepromSize() - 2;
    for (uint16_t a = 0; a < maxAddr; a++) {
        uint8_t b = EEPROM.read(a);
        crc ^= ((uint16_t)b << 8);
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}

uint16_t EEPROMStore::badgeAreaSize() const {
    return (uint16_t)UID_SIZE * (uint16_t)MAX_BADGES;
}

uint16_t EEPROMStore::eepromSize() const {
    // Use EEPROM.length() to get actual device EEPROM size at runtime
    return (uint16_t)EEPROM.length();
}