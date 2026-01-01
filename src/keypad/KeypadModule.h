#ifndef KEYPAD_MODULE_H
#define KEYPAD_MODULE_H

#include <Arduino.h>
#include <Keypad.h>
#include <EEPROM.h>

class KeypadModule {
public:
    KeypadModule(char* keysMap, byte* rowPins, byte* colPins, byte rows, byte cols, const char* defaultPIN="123");

    void begin();
    void update();                    // à appeler dans loop()
    bool isCommandReady() const;
    String getCommand();              // retourne commande complète entrée avec #

    bool checkAdminPIN(const String& pin);

private:
    Keypad keypad;
    String inputBuffer;
    String adminPIN;
    bool commandReady;
};

#endif
