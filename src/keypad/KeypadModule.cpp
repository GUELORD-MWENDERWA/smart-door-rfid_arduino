#include "KeypadModule.h"

KeypadModule::KeypadModule(char* keysMap, byte* rowPins, byte* colPins, byte rows, byte cols, const char* defaultPIN)
    : keypad(makeKeymap(keysMap), rowPins, colPins, rows, cols),
      inputBuffer(""),
      adminPIN(defaultPIN),
      commandReady(false) {}

void KeypadModule::begin() {
    keypad.addEventListener([](KeypadEvent eKey){
        // Optionnel : pour log ou debug
    });
}

void KeypadModule::update() {
    char key = keypad.getKey();
    if (key != NO_KEY) {
        if (key == '#') {
            if (inputBuffer.length() > 0) {
                commandReady = true;
            }
        } else if (key == '*') {
            inputBuffer = ""; // effacement
        } else {
            inputBuffer += key;
        }
    }
}

bool KeypadModule::isCommandReady() const {
    return commandReady;
}

String KeypadModule::getCommand() {
    commandReady = false;
    String cmd = inputBuffer;
    inputBuffer = "";
    return cmd;
}

bool KeypadModule::checkAdminPIN(const String& pin) {
    return pin.equals(adminPIN);
}
