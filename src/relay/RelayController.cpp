#include "RelayController.h"

#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#endif

RelayController::RelayController(uint8_t pin, unsigned long openTimeMs)
    : pin(pin), openTime(openTimeMs), openSince(0), state(false)
{
}

void RelayController::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    state = false;
}

void RelayController::update() {
    if (state) {
        unsigned long now = millis();
        if (openSince > 0 && (now - openSince) >= openTime) {
            // auto-close
            digitalWrite(pin, LOW);
            state = false;
            openSince = 0;
            DEBUG_PRINTLN(F("[RELAY] Auto-closed"));
        }
    }
}

void RelayController::open() {
    digitalWrite(pin, HIGH);
    state = true;
    openSince = millis();
    DEBUG_PRINTLN(F("[RELAY] Opened"));
}

void RelayController::close() {
    digitalWrite(pin, LOW);
    state = false;
    openSince = 0;
    DEBUG_PRINTLN(F("[RELAY] Closed"));
}

bool RelayController::isOpen() const {
    return state;
}