#include "RelayController.h"

RelayController::RelayController(uint8_t pin, uint16_t openDurationMs)
    : relayPin(pin),
      duration(openDurationMs),
      openTimestamp(0),
      state(false) {}

void RelayController::begin() {
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
    state = false;
}

void RelayController::open() {
    digitalWrite(relayPin, HIGH);
    state = true;
    openTimestamp = millis();
}

void RelayController::update() {
    if (state && (millis() - openTimestamp >= duration)) {
        digitalWrite(relayPin, LOW);
        state = false;
    }
}

bool RelayController::isOpen() const {
    return state;
}
