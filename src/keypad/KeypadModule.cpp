#include "KeypadModule.h"
#include "../config.h"


KeypadModule::KeypadModule(char* keysMap,
                           byte* rowPins,
                           byte* colPins,
                           byte rows,
                           byte cols,
                           const char* defaultPIN,
                           uint8_t buzzerPin)
    : keypad(makeKeymap(keysMap), rowPins, colPins, rows, cols),
      inputBuffer(""),
      adminPIN(defaultPIN),
      commandReady(false),
      attemptsLeft(MAX_ATTEMPTS),
      locked(false),
      buzzer(buzzerPin)
{
}

void KeypadModule::begin() {
    DEBUG_PRINTLN(F("[KEYPAD] Initialisation"));
    inputBuffer.reserve(16);
    resetBuffer();

    DEBUG_PRINT(F("[KEYPAD] PIN admin configuré | Tentatives="));
    DEBUG_PRINTLN(attemptsLeft);

    if (buzzer != 255) {
        pinMode(buzzer, OUTPUT);
    }
}

void KeypadModule::update() {
    if (locked) return;

    char key = keypad.getKey();
    if (!key) return;

    beep(3000, 40);

    DEBUG_PRINT(F("[KEYPAD] Touche: "));
    DEBUG_PRINTLN(key);

    if (key == '*') {
        resetBuffer();
        return;
    }

    if (key == '#') {
        commandReady = true;
        DEBUG_PRINTLN(F("[KEYPAD] Commande complète"));
        return;
    }

    inputBuffer += key;
}

bool KeypadModule::isCommandReady() const {
    return commandReady;
}

String KeypadModule::getCommand() {
    if (!commandReady) return "";

    String cmd = inputBuffer;

    DEBUG_PRINT(F("[KEYPAD] Commande lue: "));
    DEBUG_PRINTLN(cmd);

    resetBuffer();
    return cmd;
}

bool KeypadModule::checkAdminPIN(String pin) {
    if (locked) return false;

    pin.trim();

    DEBUG_PRINT(F("[KEYPAD] Vérification PIN: "));
    DEBUG_PRINTLN(pin);

    if (pin == adminPIN) {
        DEBUG_PRINTLN(F("[KEYPAD] PIN OK"));
        attemptsLeft = MAX_ATTEMPTS;
        beep(2000, 150);
        resetBuffer();
        return true;
    }

    attemptsLeft--;
    beep(400, 300);

    DEBUG_PRINT(F("[KEYPAD] PIN incorrect | Tentatives restantes: "));
    DEBUG_PRINTLN(attemptsLeft);

    resetBuffer();

    if (attemptsLeft == 0) {
        locked = true;
        DEBUG_PRINTLN(F("[KEYPAD] CLAVIER BLOQUÉ"));
    }

    return false;
}

/* ===== CORRECTION MINIMALE =====
   Empêcher l'écrasement du PIN admin par une valeur vide
*/
bool KeypadModule::changeAdminPIN(const String& newPin) {
    if (newPin.length() < 3 || newPin.length() > 8) {
        DEBUG_PRINTLN(F("[KEYPAD] Nouveau PIN invalide"));
        return false;
    }

    if (newPin.length() == 0) {
        DEBUG_PRINTLN(F("[KEYPAD] PIN vide ignoré"));
        return false;
    }

    adminPIN = newPin;
    DEBUG_PRINT(F("[KEYPAD] Nouveau PIN admin défini: "));
    DEBUG_PRINTLN(adminPIN);

    return true;
}

uint8_t KeypadModule::getRemainingAttempts() const {
    return attemptsLeft;
}

bool KeypadModule::isLocked() const {
    return locked;
}

/* ===== PRIVATE ===== */

void KeypadModule::resetBuffer() {
    inputBuffer = "";
    commandReady = false;
}

void KeypadModule::beep(uint16_t freq, uint16_t duration) {
    if (buzzer == 255) return;
    tone(buzzer, freq, duration);
}