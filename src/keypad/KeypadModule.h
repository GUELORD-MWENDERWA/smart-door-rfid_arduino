#ifndef KEYPAD_MODULE_H
#define KEYPAD_MODULE_H

#include <Arduino.h>
#include <Keypad.h>

/*
  KeypadModule
  - Simple wrapper around Keypad library.
  - NOTE: currently uses String for compatibility with existing main.cpp.
    If memory constraints force it, we can refactor to c-strings later.
*/

class KeypadModule {
public:
    static const uint8_t MAX_ATTEMPTS = 5;

    KeypadModule(char* keysMap,
                 byte* rowPins,
                 byte* colPins,
                 byte rows,
                 byte cols,
                 const char* defaultPIN = "123",
                 uint8_t buzzerPin = 255);

    void begin();
    void update();

    bool isCommandReady() const;
    String getCommand();                  // retourne la commande SANS '#'

    bool checkAdminPIN(String pin);       // pin attendu sans '#'
    bool changeAdminPIN(const String& newPin);

    uint8_t getRemainingAttempts() const;
    bool isLocked() const;

private:
    Keypad keypad;

    String inputBuffer;
    String adminPIN;

    bool commandReady;
    uint8_t attemptsLeft;
    bool locked;

    uint8_t buzzer;

    void resetBuffer();
    void beep(uint16_t freq, uint16_t duration);
};

#endif