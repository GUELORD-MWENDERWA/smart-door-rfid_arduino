#include "EEPROMStore.h"

void EEPROMStore::begin() {
    if (EEPROM.read(EEPROM_MAGIC_ADDR) != EEPROM_MAGIC) {
        reset();
        EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
    }

    badgeCount = EEPROM.read(EEPROM_COUNT_ADDR);
    if (badgeCount > MAX_BADGES) {
        reset();
    }
}

uint8_t EEPROMStore::getBadgeCount() {
    return badgeCount;
}

int EEPROMStore::badgeAddress(uint8_t index) {
    return EEPROM_BASE_ADDR + (index * UID_SIZE);
}

bool EEPROMStore::compareUID(const uint8_t *a, const uint8_t *b) {
    for (uint8_t i = 0; i < UID_SIZE; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool EEPROMStore::badgeExists(const uint8_t *uid) {
    for (uint8_t i = 0; i < badgeCount; i++) {
        uint8_t stored[UID_SIZE];
        for (uint8_t j = 0; j < UID_SIZE; j++) {
            stored[j] = EEPROM.read(badgeAddress(i) + j);
        }
        if (compareUID(uid, stored)) return true;
    }
    return false;
}

bool EEPROMStore::addBadge(const uint8_t *uid) {
    if (badgeCount >= MAX_BADGES) return false;
    if (badgeExists(uid)) return false;

    int addr = badgeAddress(badgeCount);
    for (uint8_t i = 0; i < UID_SIZE; i++) {
        EEPROM.write(addr + i, uid[i]);
    }

    badgeCount++;
    EEPROM.write(EEPROM_COUNT_ADDR, badgeCount);
    return true;
}

bool EEPROMStore::removeBadge(const uint8_t *uid) {
    for (uint8_t i = 0; i < badgeCount; i++) {
        uint8_t stored[UID_SIZE];
        for (uint8_t j = 0; j < UID_SIZE; j++) {
            stored[j] = EEPROM.read(badgeAddress(i) + j);
        }

        if (compareUID(uid, stored)) {
            // Décalage des badges suivants
            for (uint8_t k = i; k < badgeCount - 1; k++) {
                for (uint8_t j = 0; j < UID_SIZE; j++) {
                    EEPROM.write(
                        badgeAddress(k) + j,
                        EEPROM.read(badgeAddress(k + 1) + j)
                    );
                }
            }

            badgeCount--;
            EEPROM.write(EEPROM_COUNT_ADDR, badgeCount);
            return true;
        }
    }
    return false;
}

void EEPROMStore::reset() {
    badgeCount = 0;
    EEPROM.write(EEPROM_COUNT_ADDR, 0);

    for (int i = EEPROM_BASE_ADDR;
         i < EEPROM_BASE_ADDR + (MAX_BADGES * UID_SIZE);
         i++) {
        EEPROM.write(i, 0x00);
    }
}
