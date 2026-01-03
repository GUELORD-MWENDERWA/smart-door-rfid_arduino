#include "EEPROMStore.h"

/* ===== EEPROM LAYOUT =====
   [MAGIC]
   [BADGE_COUNT]
   [ADMIN_PIN ...]
   [BADGES ...]
*/

void EEPROMStore::begin() {
    Serial.println(F("[EEPROM] Initialisation"));

    if (EEPROM.read(EEPROM_MAGIC_ADDR) != EEPROM_MAGIC) {
        Serial.println(F("[EEPROM] Magic incorrect -> RESET"));
        reset();
        EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
        writeAdminPIN(DEFAULT_ADMIN_PIN);
    }

    badgeCount = EEPROM.read(EEPROM_COUNT_ADDR);
    if (badgeCount > MAX_BADGES) {
        Serial.println(F("[EEPROM] Badge count invalide -> RESET"));
        reset();
        writeAdminPIN(DEFAULT_ADMIN_PIN);
    }

    /* ===== CORRECTION CRITIQUE =====
       S'assurer que le PIN admin n'est jamais vide ou invalide
    */
    String pin = readAdminPIN();
    if (pin.length() == 0) {
        Serial.println(F("[EEPROM] PIN admin manquant -> PIN par défaut"));
        writeAdminPIN(DEFAULT_ADMIN_PIN);
    }

    Serial.print(F("[EEPROM] Badges chargés: "));
    Serial.println(badgeCount);
}

/* ===== ADMIN PIN ===== */

void EEPROMStore::writeAdminPIN(const String& pin) {
    for (uint8_t i = 0; i < ADMIN_PIN_LEN; i++) {
        EEPROM.write(
            EEPROM_ADMIN_PIN_ADDR + i,
            (i < pin.length()) ? pin[i] : '\0'
        );
    }
}

String EEPROMStore::readAdminPIN() {
    char buf[ADMIN_PIN_LEN + 1];
    uint8_t len = 0;

    for (uint8_t i = 0; i < ADMIN_PIN_LEN; i++) {
        char c = EEPROM.read(EEPROM_ADMIN_PIN_ADDR + i);
        if (c == '\0') break;
        buf[len++] = c;
    }

    buf[len] = '\0';
    return String(buf);
}

/* ===== BADGES ===== */

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

void EEPROMStore::debugUID(const uint8_t *uid) {
    for (uint8_t i = 0; i < UID_SIZE; i++) {
        if (uid[i] < 0x10) Serial.print('0');
        Serial.print(uid[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
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

/* ===== RESET ===== */

void EEPROMStore::reset() {
    badgeCount = 0;
    EEPROM.write(EEPROM_COUNT_ADDR, 0);

    /* ===== CORRECTION =====
       Le reset doit aussi réinitialiser le PIN admin
    */
    writeAdminPIN(DEFAULT_ADMIN_PIN);

    for (int i = EEPROM_BASE_ADDR;
         i < EEPROM_BASE_ADDR + (MAX_BADGES * UID_SIZE);
         i++) {
        EEPROM.write(i, 0x00);
    }
}
