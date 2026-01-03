#include "KeypadModule.h"

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
    Serial.println(F("[KEYPAD] Initialisation"));
    inputBuffer.reserve(16);
    resetBuffer();

    Serial.print(F("[KEYPAD] PIN admin configuré | Tentatives="));
    Serial.println(attemptsLeft);

    if (buzzer != 255) {
        pinMode(buzzer, OUTPUT);
    }
}

void KeypadModule::update() {
    if (locked) return;

    char key = keypad.getKey();
    if (!key) return;

    beep(3000, 40);

    Serial.print(F("[KEYPAD] Touche: "));
    Serial.println(key);

    if (key == '*') {
        resetBuffer();
        return;
    }

    if (key == '#') {
        commandReady = true;
        Serial.println(F("[KEYPAD] Commande complète"));
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

    Serial.print(F("[KEYPAD] Commande lue: "));
    Serial.println(cmd);

    resetBuffer();
    return cmd;
}

bool KeypadModule::checkAdminPIN(String pin) {
    if (locked) return false;

    pin.trim();

    Serial.print(F("[KEYPAD] Vérification PIN: "));
    Serial.println(pin);

    if (pin == adminPIN) {
        Serial.println(F("[KEYPAD] PIN OK"));
        attemptsLeft = MAX_ATTEMPTS;
        beep(2000, 150);
        resetBuffer();
        return true;
    }

    attemptsLeft--;
    beep(400, 300);

    Serial.print(F("[KEYPAD] PIN incorrect | Tentatives restantes: "));
    Serial.println(attemptsLeft);

    resetBuffer();

    if (attemptsLeft == 0) {
        locked = true;
        Serial.println(F("[KEYPAD] CLAVIER BLOQUÉ"));
    }

    return false;
}

/* ===== CORRECTION MINIMALE =====
   Empêcher l'écrasement du PIN admin par une valeur vide
*/
bool KeypadModule::changeAdminPIN(const String& newPin) {
    if (newPin.length() < 3 || newPin.length() > 8) {
        Serial.println(F("[KEYPAD] Nouveau PIN invalide"));
        return false;
    }

    if (newPin.length() == 0) {
        Serial.println(F("[KEYPAD] PIN vide ignoré"));
        return false;
    }

    adminPIN = newPin;
    Serial.print(F("[KEYPAD] Nouveau PIN admin défini: "));
    Serial.println(adminPIN);

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
